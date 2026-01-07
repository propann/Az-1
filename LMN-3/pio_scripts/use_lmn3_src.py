import os
from pathlib import Path

Import("env")  # type: ignore

project_dir = Path(env["PROJECT_DIR"])
src_dir = project_dir / "LMN-3" / "src"
include_dir = project_dir / "LMN-3" / "include"

# Point this environment at the LMN-3 sources and includes
env.Replace(PROJECT_SRC_DIR=str(src_dir))
env.Replace(PROJECTSRC_DIR=str(src_dir))  # compatibility alias
env.Replace(SRC_BUILD_FLAGS=["+<*>"])
env.Replace(SRC_FILTER=["+<*>"])
env.Replace(PROJECT_INCLUDE_DIR=str(include_dir))
env.Append(CPPPATH=[str(include_dir)])
print(f"[LMN-3] Using src_dir={src_dir} include_dir={include_dir}")
