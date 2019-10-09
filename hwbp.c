#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>

#include <assert.h>
#include <sys/debugreg.h>

extern int errno;


// Copy the 'num_bits' LSBs frm val into the indicated bit-range of 'input'. Return the
// result.
static uint32_t set_bits(uint32_t input, int input_offset, int num_bits, uint8_t val) {
    assert(num_bits > 0);
    assert(num_bits <= 8);
    assert(input_offset >= 0);
    assert(input_offset < 32);
    assert(num_bits + input_offset < 32);

    uint32_t result = input;

    // Clear the existing bits in the specified range...
    uint8_t clear_byte_mask = 0x0;
    for (int i = 0; i < num_bits; ++i) {
        clear_byte_mask = (clear_byte_mask << 1) | 0x1;
    }

    const uint32_t clear_word_mask = ((uint32_t)clear_byte_mask) << input_offset;
    result = result & ~clear_word_mask;

    // Set the bits to their new value...
    const uint32_t set_word_mask = ((uint32_t)val) << input_offset;
    result = result | set_word_mask;

    return result;
}

static uint32_t set_local_breakpoint_enabled(uint32_t old_control, int bpno) {
    const int bit_offset = DR_LOCAL_ENABLE_SHIFT + bpno * DR_ENABLE_SIZE;
    const int num_bits = 1;
    const uint8_t val = 0x1;

    return set_bits(old_control, bit_offset, num_bits, val);
}

static uint32_t set_breakpoint_len(uint32_t old_control, int bpno, int num_bytes) {
    uint8_t val = 0x0;
    switch (num_bytes) {
        case 1:
            val = DR_LEN_1;
            break;
        case 2:
            val = DR_LEN_2;
            break;
        case 4:
            val = DR_LEN_4;
            break;
        case 8:
            val = DR_LEN_8;
            break;
        default:
            assert(! "Invalid watchpoint length.");
            return old_control;
    };

    const int bit_offset = DR_CONTROL_SHIFT + (DR_CONTROL_SIZE * bpno) + 2;
    const int num_bits = 2;

    return set_bits(old_control, bit_offset, num_bits, val);
}

static uint32_t set_breakpoint_kind(uint32_t old_control, int bpno, bool code_execute, bool data_read, bool data_write) {
    uint8_t val;
    if (code_execute && (!data_read) && (!data_write)) {
        val = DR_RW_EXECUTE;
    }
    else if ((!code_execute) && (!data_read) && (data_write)) {
        val = DR_RW_WRITE;
    }
    else if ((!code_execute) && (data_read) && (data_write)) {
        val = DR_RW_READ;
    }
    else {
        assert(! "Invalid breakpoint kind.");
        return old_control;
    }

    const int bit_offset = DR_CONTROL_SHIFT + (DR_CONTROL_SIZE * bpno);
    const int num_bits = 2;

    return set_bits(old_control, bit_offset, num_bits, val);
}

/*
 * This function fork()s a child that will use
 * ptrace to set a hardware breakpoint for
 * memory r/w at _addr_. When the breakpoint is
 * hit, then _handler_ is invoked in a signal-
 * handling context.
 */
bool install_breakpoint(void *addr, int num_bytes, int bpno, void (*handler)(int)) {
        if ((bpno < DR_FIRSTADDR) || (bpno > DR_LASTADDR)) {
            fprintf(stderr, "bpno is out of range: %d\n", bpno);
            return false;
        }

	pid_t child = 0;

        // FIXME(cconvey): In an ideal world, we'd start with the current value of DR7, rather than
        // 0. Because of our shortcut, this function will destroy all existing hardware
        // breakpoints.
        uint32_t new_control_reg = 0;
        new_control_reg = set_breakpoint_kind(new_control_reg, bpno, false, false, true);
        new_control_reg = set_breakpoint_len(new_control_reg, bpno, num_bytes);
        new_control_reg = set_local_breakpoint_enabled(new_control_reg, bpno);

	pid_t parent = getpid();
	int child_status = 0;

	if (!(child = fork()))
	{
		int parent_status = 0;
		if (ptrace(PTRACE_ATTACH, parent, NULL, NULL))
			_exit(1);

		while (!WIFSTOPPED(parent_status))
			waitpid(parent, &parent_status, 0);

		/*
		 * set the breakpoint address.
		 */
		if (ptrace(PTRACE_POKEUSER,
		           parent,
		           offsetof(struct user, u_debugreg[bpno]),
		           addr))
			_exit(1);

		/*
		 * set parameters for when the breakpoint should be triggered.
		 */
		if (ptrace(PTRACE_POKEUSER,
		           parent,
		           offsetof(struct user, u_debugreg[DR_CONTROL]),
                           new_control_reg))
                {
			_exit(1);
                }

		if (ptrace(PTRACE_DETACH, parent, NULL, NULL))
                {
			_exit(1);
                }

		_exit(0);
	}

	waitpid(child, &child_status, 0);

        // FIXME: It doesn't make sense to have an API that suggests different handlers for different breakpoints.
	signal(SIGTRAP, handler);

	if (WIFEXITED(child_status) && !WEXITSTATUS(child_status))
		return true;
	return false;
}

/*
 * This function will disable a breakpoint by
 * invoking install_breakpoint is a 0x0 _addr_
 * and no handler function. See comments above
 * for implementation details.
 */
bool disable_breakpoint(int bpno)
{
    // FIXME: This isn't merely disabling the breakpoint it's also messing with other data.
    // And with other limitations of install_breakpoint(...), this function actually deletes
    // all local breakpoints.
	return install_breakpoint(0x0, 1, bpno, NULL);
}
