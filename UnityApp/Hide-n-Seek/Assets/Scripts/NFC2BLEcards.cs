using UnityEngine;
using System.Collections;

public class NFC2BLEcards : MonoBehaviour 
{
	public UnityEngine.UI.Text errorText;

	private byte[] data;
	private BluetoothDeviceScript bluetoothDeviceScript;
	
	string service_uuid = "2220";
	

	// Use this for initialization
	void Start () 
	{
        DontDestroyOnLoad(transform.gameObject);
		//errorText.gameObject.SetActive(false);
		StartBLE();
	}
	

	public void StartBLE(){
		BluetoothLEHardwareInterface.Log("BLE: initalizing");
		bluetoothDeviceScript = BluetoothLEHardwareInterface.Initialize (true, false, InitializedAction, ErrorAction);
	}

	// Callbacks (Actions/Delegates):
	public void InitializedAction (){
		BluetoothLEHardwareInterface.Log("BLE: initialized");
		BluetoothLEHardwareInterface.ScanForPeripheralsWithServices (new string[] { service_uuid }, DiscoveredPeripheralAction);
	}

	public void ErrorAction (string error){
		errorText.gameObject.SetActive(true);
		errorText.text = error;
		BluetoothLEHardwareInterface.Log("BLE: initialization error: " + error);
	}
	
	public void DiscoveredPeripheralAction(string identifier, string name){
		BluetoothLEHardwareInterface.Log("BLE: discovered device: identifier=" + identifier + ", name="+name);
		if(name == "woopibot"){
			BluetoothLEHardwareInterface.Log("BLE: Connecting to " + identifier);
			BluetoothLEHardwareInterface.ConnectToPeripheral(identifier, ConnectedPeripheralAction, DiscoveredServiceAction, DiscoveredCharacteristicAction);
		}
	}

	public void ConnectedPeripheralAction(string identifier){
		BluetoothLEHardwareInterface.Log("BLE: connected to " + identifier);


	}

	public void DiscoveredServiceAction(string identifier, string serviceUUID){
		BluetoothLEHardwareInterface.Log("BLE: discovered service of " + identifier + ", serviceUUID: " + serviceUUID);
	}

	public void DiscoveredCharacteristicAction(string identifier, string serviceUUID, string characteristicUUID){
		BluetoothLEHardwareInterface.Log("BLE: discovered characteristic of " + identifier + " with serviceUUID " + serviceUUID + ", characteristicUUID: " + characteristicUUID);
	
	
		if(characteristicUUID == "2221"){
			BluetoothLEHardwareInterface.Log("BLE: found receive characteristic, subscribing to notifications");
			BluetoothLEHardwareInterface.SubscribeCharacteristic (identifier, serviceUUID, characteristicUUID, DidUpdateNotifiationStateForCharacteristicAction, DidUpdateCharacteristicValueAction);
		}
	}

	public void DidUpdateNotifiationStateForCharacteristicAction(string characteristicUUID){
		BluetoothLEHardwareInterface.Log("BLE: DidUpdateNotifiationStateForCharacteristicAction, characteristicUUID: " + characteristicUUID);
	}

	public void DidUpdateCharacteristicValueAction(string characteristicUUID, byte[] data){

		// TODO dirty fix: use characteristicUUID as unique identifier:
		onProximity(characteristicUUID, data);
	}

	public void onNewBracelet(string braceletId)
    {
		BluetoothLEHardwareInterface.Log("BLE: New Bracelet detected with id=" + braceletId);

        /*Check in setup scene by searching for the setupscript component*/
        //TODO find clean way to do this :)
        GameObject obj = GameObject.Find("_scripts");
	    if (obj == null)
	        return;

	    SetupSceneScript ssComp = obj.GetComponent<SetupSceneScript>();
	    if (ssComp == null)
	        return;

        GameManager.Instance.onNewBracelet(braceletId);
    }

	public void onCard(string braceletId, byte[] uid)
	{
		Debug.Log ("trestsdsfdsfsd");


		string uidHexString = "";
		foreach (byte b in uid){
			uidHexString += string.Format("{0:X2}", b);
		}
		

		string card = "UNKNOWN";
		int id = 0x0001;
		
		switch(uidHexString)
		{
		case "04904BAAF52680": 	id = 0x02; card = "banana"; break;
		case "0483A7AAF52680": 	id = 0x04; card = "fire"; break;
		case "04B1A7AAF52680": 	id = 0x08; card = "canoe"; break;
		}

		BluetoothLEHardwareInterface.Log("BLE: Bracelet with id=" + braceletId + " sends " + uidHexString + ", which is " + card);

		
		GameManager.Instance.ScannedCard(braceletId, id);
	}

	public void onProximity(string braceletId, byte[] uid)
	{
		string uidHexString = "";
		for (int i = 1; i < uid.Length-1; i++){
			int x = uid[i];
			uidHexString+= x;
		}

//		foreach (byte b in uid){
//			uidHexString += string.Format("{0:X2}", b);
//		}
		int id = 0x0001;

		
		BluetoothLEHardwareInterface.Log("BLE: proximity=" + uidHexString);

		GameObject obj = GameObject.Find("_scripts");
		if (obj == null)
			return;
		
		SetupSceneScript ssComp = obj.GetComponent<SetupSceneScript>();
		if (ssComp == null)
			return;
		
		ssComp.proximityValues(uidHexString);
	}
}
