/**
 * @brief Process manager
 */

#ifndef PROCESS_MGR_H_
#define PROCESS_MGR_H_

/// @todo put this in a posix header
typedef unsigned int pid_t;

/**
 * @brief Process manager
 */
class ProcessMgr
{
public:
    /**
     * @brief Constructor
     */
    ProcessMgr();

    void createProcess();

private:
    struct ProcessInfo
    {
        /// Unique ID for the process
        pid_t id;
    };

    constexpr static int PROCESS_INFO_SIZE = 4;
    ProcessInfo processInfo[PROCESS_INFO_SIZE];
};

#endif // PROCESS_MGR_H_
