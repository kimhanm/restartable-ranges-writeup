


# `00_hello_ranges/`
In this program, we define a dummy restartable range in `ranges.S` and construct an appropriate range which is then registered with `task_restartable_ranges_register`.

If we try running this program, it will fail as the kernel responds with `KERN_NOT_SUPPORTED (46)`.
To figure out why, let us start up a debugger, insert a breakpoint at `main` and at `task_restartable_ranges_register` and follow execution.
```sh
make inspect        # for the lazy
```
Note that even _before_ we enter `main`, the `task_restartable_ranges_register` breakpoint is triggered by `dyld>libSystem>libdispatch>libobjc`
If we read the source code, we see that release kernels may only register restartable ranges once <https://github.com/apple-oss-distributions/xnu/blob/main/osfmk/kern/restartable.c#L562> in their lifetime.

Our takeaway is that if we want to use restartable ranges in our own program, we must somehow take over their registration and append our ranges to that list.

