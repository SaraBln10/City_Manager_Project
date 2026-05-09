# AI Usage Documentation — Phases 1 and 2

## Tool Used
Claude (claude-sonnet-4-6), accessed via claude.ai

---

# Phase 1

## Prompt 1 — parse_condition

**What I asked:**
> I have a C struct named Report containing these fields:
> - id (int)
> - inspector (char[64])
> - latitude (double)
> - longitude (double)
> - category (char[32])
> - severity (int)
> - timestamp (time_t)
> - description (char[256])
>
> Write a function:
>   int parse_condition(const char *input, char *field, char *op, char *value);
> that receives a string in the format field:operator:value and separates it
> into its three components. The supported operators are ==, !=, <, <=, >, >=.

**What was generated:**
The AI produced a function that used `strtok()` to tokenize the input string
by splitting on `:` characters. It correctly handled multi-character operators
such as `<=` and `>=` by treating them as strings rather than individual characters.

**What I changed:**
`strtok()` is destructive — it modifies the input string in place by replacing
delimiters with null bytes. Since the input string comes from `argv[]`, which
should remain unmodified, I replaced the implementation with pointer arithmetic
using `strchr()`. This approach locates the colon separators without altering
the original string.

---

## Prompt 2 — match_condition

**What I asked:**
> Using the same Report struct described above, write a function:
>   int match_condition(Report *r, const char *field, const char *op, const char *value);
> that returns 1 if the given report satisfies the condition, and 0 otherwise.
> Supported fields: severity, category, inspector, timestamp.
> Supported operators: ==, !=, <, <=, >, >=.

**What was generated:**
The AI generated a function that separated the logic into two branches —
string fields (category, inspector) handled with `strcmp()`, and numeric
fields (severity, timestamp) handled by converting the value string with `atoi()`
before performing the comparison.

**What I changed:**
The use of `atoi()` for the timestamp field was incorrect. On 64-bit Linux systems,
`time_t` is a 64-bit value, and `atoi()` returns a 32-bit int, which would silently
truncate large timestamp values. I replaced it with `atol()` to correctly handle
the full range of Unix timestamps.

Additionally, the original generated code returned 0 silently for unknown fields
and unsupported operators with no diagnostic output. I added `fprintf(stderr, ...)`
warnings in both cases to make errors visible during testing and debugging.

---

## What I wrote myself — Phase 1

The `filter_reports()` function was written entirely without AI assistance,
as required by the project specification. It:
- Opens `reports.dat` using the `open()` system call
- Reads fixed-size records sequentially using `read()`
- Parses all conditions upfront using `parse_condition()` to detect format errors early
- Tests each record against every condition using `match_condition()`
- Prints only records where all conditions return 1 (AND logic)

---

## What I learned — Phase 1

- `strtok()` mutates the input string, making it unsuitable for read-only data
  like command-line arguments. Pointer-based parsing with `strchr()` is safer.
- `time_t` is 64 bits on modern Linux, so `atoi()` is the wrong tool for
  timestamp conversion — `atol()` must be used instead.
- Reviewing AI-generated code line by line revealed subtle type and safety issues
  that would have caused hard-to-debug problems at runtime.
- The value of the AI-assisted exercise was not in getting working code, but in
  being forced to understand and critically evaluate every line before using it.

---

# Phase 2

## AI Usage in Phase 2

No AI-generated code was used in Phase 2. All implementation was written
independently, including:
- `district_remove()` in `district.c` — fork/exec pattern with `waitpid()`
- `monitor_main.c` — signal handling with `sigaction()`, PID file management
- `monitor.c` — reading `.monitor_pid` and sending SIGUSR1 via `kill()`
- Changes to `main.c` — `remove_district` command and monitor notification on add

## AI consulted for — conceptual clarification

AI was used to clarify two concepts before writing the code:

**1. sigaction() vs signal()**
> Why does the spec explicitly forbid signal() and require sigaction()?

The AI explained that `signal()` has implementation-defined behavior in POSIX
and is unsafe when called from within a signal handler. `sigaction()` is the
correct POSIX interface, offering explicit control over signal masks, handler
flags such as `SA_RESTART`, and consistent behavior across all UNIX platforms.
This understanding was applied when implementing both the SIGUSR1 and SIGINT
handlers in `monitor_main.c`.

**2. Waiting for signals efficiently**
> What is the right way to wait for signals without consuming CPU?

The AI explained that `pause()` suspends the calling process until a signal
is delivered, making it the correct approach for a signal-driven wait loop.
The `while (running) pause()` pattern in `monitor_main.c` was written based
on this understanding.

## What I wrote myself — Phase 2

All code in Phase 2 was written independently. AI was consulted only for
conceptual understanding, not for generating any functions or logic.

## What I learned — Phase 2

- The `fork()` + `execl()` combination is the standard UNIX pattern for
  spawning external processes. The parent process must call `waitpid()` to
  collect the child's exit status and prevent zombie processes.
- `sigaction()` provides much more control than `signal()`, especially the
  `SA_RESTART` flag which prevents interrupted system calls from failing
  with EINTR.
- Using `kill()` with a PID stored in a file is a simple and effective way
  to communicate between two unrelated processes.
- The pattern of writing a PID file on startup and removing it on exit is
  a standard UNIX convention used by real-world daemons and services.