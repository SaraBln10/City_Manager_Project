# AI Usage Documentation — Phases 1, 2 and 3

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

---

# Phase 3

## AI Usage in Phase 3

No AI-generated code was produced with AI assistance in Phase 3. Everything
was implemented independently, covering:
- `city_hub.c` — interactive terminal interface handling `start_monitor`
  and `calculate_scores` commands
- `scorer.c` — standalone program that reads district reports and computes
  a workload score per inspector
- Updated `monitor_main.c` — detects a running monitor instance on startup
  and uses a structured output format for pipe-based communication

## AI consulted for — conceptual clarification

Two concepts were clarified with AI before implementation:

**1. Combining pipe() and dup2() to capture child output**
> How do you set up a pipe so that a child process's stdout is readable
> by the parent?

The AI clarified that `pipe()` produces two descriptors — one for reading
and one for writing. After `fork()`, the child replaces its stdout with the
write end using `dup2(pipefd[1], STDOUT_FILENO)`, then closes both raw
descriptors. The parent closes the write end and reads from the read end.
This pattern was applied in both `run_hub_mon()` and `run_calculate_scores()`
inside `city_hub.c`.

**2. Distinguishing message types over a pipe**
> What is a simple way for a parent to tell apart different kinds of messages
> arriving from a child through a pipe?

The AI suggested using a fixed string prefix per message type — for example
`MSG:` for informational output and `ERR:` for error conditions. The parent
reads line by line and branches on the prefix. This approach was used to
modify `monitor_main.c` output and parse it correctly in `city_hub.c`.

## What I wrote myself — Phase 3

Every function and file in Phase 3 was written without AI code generation.
AI was only used to understand the underlying concepts before sitting down
to write the implementation.

## What I learned — Phase 3

- `dup2()` replaces a file descriptor with another — calling it before
  `exec()` in the child means the new program writes to the pipe without
  knowing it, since it simply writes to stdout as usual.
- Forgetting to close the write end of the pipe in the parent is a common
  bug — the parent will never see EOF on the read end and will block
  indefinitely waiting for data that never arrives.
- A lightweight prefix-based protocol is sufficient for distinguishing
  message categories over a pipe — no need for complex serialization.
- `fdopen()` wraps a raw file descriptor in a `FILE*`, enabling `fgets()`
  for convenient line-by-line reading, which is far simpler than managing
  partial reads manually with `read()`.
