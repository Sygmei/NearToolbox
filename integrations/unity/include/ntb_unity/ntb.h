#pragma once

extern "C"
{

    void *ntb_near_client_create(const char *network, const char *private_key, const char *account_id);
    void ntb_near_client_transfer(void *near_client_ptr, const char *recipient, const char *amount);
    void ntb_near_client_delete(void *near_client_ptr);

    // Async
    struct Job
    {
        bool success = false;
        bool done = false;
    };

    void ntb_async_loop();
    Job *ntb_near_client_transfer_async(void *near_client_ptr, const char *recipient, const char *amount);
}
