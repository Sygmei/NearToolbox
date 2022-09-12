extends Node

func _ready():
	$NearClient.login();

    # Transfering some funds
    $NearClient.transfer("your_address.testnet", "0.5");

    # Calling a smart contract
    var payload = {"message": "Hello NEARCON :)"};
	var payload_json = JSON.stringify(payload);
    $NearClient.contract_call("akc.nearcon22.testnet", "set_greeting", payload_json, "0");
