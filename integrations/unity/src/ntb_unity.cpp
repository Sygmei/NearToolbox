#include <atomic>
#include <thread>

#include <ntb_unity/ntb.h>

#include <ntb/near.hpp>

void *ntb_near_client_create(const char *network, const char *private_key, const char *account_id)
{
    ntb::ED25519Keypair signer{private_key};
    ntb::NamedAccount account_id_wrapper{std::string(account_id)};
    return new ntb::NearClient(network, std::move(signer), account_id_wrapper);
}

void ntb_near_client_transfer(void *near_client_ptr, const char *recipient, const char *amount)
{
    ntb::NearClient *near_client = static_cast<ntb::NearClient *>(near_client_ptr);
    near_client->transfer(recipient, amount);
}

void ntb_near_client_delete(void *near_client_ptr)
{
    ntb::NearClient *near_client = static_cast<ntb::NearClient *>(near_client_ptr);
    delete near_client;
}

struct JobExecution
{
    Job result;
    std::thread thr;
};

static std::vector<std::unique_ptr<JobExecution>> JOBS;

template <class Func, class... Args>
JobExecution *make_job(Func func, Args &&...args)
{
    std::unique_ptr<JobExecution> job = std::make_unique<JobExecution>();
    job->thr = std::thread([&]()
                           {
        try
        {
            func(std::forward<Args>(args)...);
            job->result.success = true;
            job->result.done = true;
        }
        catch (...)
        {
            job->result.success = false;
            job->result.done = true;
        } });
    JOBS.push_back(std::move(job));
    return job.get();
}

void ntb_async_loop()
{
    for (const auto &job : JOBS)
    {
    }
}

Job *ntb_near_client_transfer_async(void *near_client_ptr, const char *recipient, const char *amount)
{
    return &make_job(ntb_near_client_transfer, near_client_ptr, recipient, amount)->result;
}