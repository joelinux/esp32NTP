Import("env")

# Get the custom prefix from platformio.ini
prefix = env.GetProjectOption("custom_firmware_prefix")

# Rename firmware (main application binary)
env.Replace(PROGNAME=f"{prefix}_firmware")

# Post-build actions to rename bootloader and partitions
def rename_binaries(source, target, env):
    import os
    build_dir = env.subst("$BUILD_DIR")
    
    # Rename bootloader.bin
    bootloader_src = f"{build_dir}/bootloader.bin"
    bootloader_dst = f"{build_dir}/{prefix}_bootloader.bin"
    env.Execute(f'cp "{bootloader_src}" "{bootloader_dst}"')
    env.Execute(f'rm "{bootloader_src}"')
    
    # Rename partitions.bin
    partitions_src = f"{build_dir}/partitions.bin"
    partitions_dst = f"{build_dir}/{prefix}_partitions.bin"
    if os.path.exists(partitions_src):
        env.Execute(f'cp "{partitions_src}" "{partitions_dst}"')
        env.Execute(f'rm "{partitions_src}"')

# Add post-build action
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", rename_binaries)
