using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System;
using UnityEngine;

public class NearScript : MonoBehaviour
{
    [DllImport("libneartoolbox_unity")]
    public static extern IntPtr ntb_near_client_create(string network, string private_key, string account_id);
    [DllImport("libneartoolbox_unity")]
    public static extern void ntb_near_client_transfer_async(IntPtr near_client_ptr, string recipient, string amount);

    private IntPtr near_client;

    void Start()
    {
        print("Creating client...");
        near_client = ntb_near_client_create("testnet", "your_private_key", "your_account.testnet");

        Transfer("your_recipient.testnet", "0.145");
    }

    void Transfer(string recipient, string amount)
    {
        print("Transfering money");
        ntb_near_client_transfer_async(near_client, recipient, amount);
    }

    void Update()
    {
    }
}
