#pragma once

/**
 * TODO: Pick up here with defining the filesystem class
 * - Since we're C++, use inheritance instead of C-style
 *   structs with function pointers for 'interfaces'
 *
 * TODO: mount/umount will likely be the first programs to drive this code
 *
 * - keep in mind the concept of namespaces is really cool...obviously
 *   we want to support chroot, but making this nice and generic might
 *   allow us to implement some cool container software allowing users
 *   to run "VMs" on the native machine with root access in their sandbox
 *   environment (a filesystem namespace)
 */
class Filesystem
{

};

/**
 * @brief Registers the filesystem with the OS. Filesystems can only be
 * registered once regardless of how many instances there may be.
 *
 * - TODO: do filesystem names serve as ids?
 * - TODO: return codes/error conditions
 *
 * @param fs is the filesystem to be registered
 * @return int
 */
int registerFilesystem(Filesystem& fs);
