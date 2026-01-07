import subprocess

Import("env")  # type: ignore


def git_sha():
    try:
        return (
            subprocess.check_output(["git", "rev-parse", "--short", "HEAD"])
            .decode("utf-8")
            .strip()
        )
    except Exception:
        return "UNKNOWN"


env.Append(CPPDEFINES=[("GIT_SHA", f'"{git_sha()}"')])
