using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Text;

public class BLERobot
{
	private string characteristicUUID = "2221";

	public string id;  // in uuid form, eg: 92B314BC-B02E-4926-B8E8-D5B48D69B9A6
	public string name;
	public int rssi;

	public bool isConnected = false;

	
	private List<Byte> receivingCommandbytes = new List<Byte>();
	private bool receivingCommandStarted = false;
			
			
	public BLERobot (string name, string id)
	{
		this.name = name;
		this.id = id;

		Debug.Log ("new robot object created");
	}

	public void Connect()
	{
		Debug.Log ("BLE - " + this.id + " : connecting");
		BluetoothLEHardwareInterface.ConnectToPeripheral(this.id, ConnectedPeripheralAction, DiscoveredServiceAction, DiscoveredCharacteristicAction, DisconnectedPeripheralAction);
	}

	public void ConnectedPeripheralAction(string identifier)
	{
		if (!isConnected) {
			Debug.Log ("BLE - " + this.id + " : connected");
			isConnected = true;
		}
	}

	public void DisconnectedPeripheralAction(string identifier)
	{
		isConnected = false;

		Debug.Log ("BLE - " + this.id + " : disconnected!");

		GameManager.Instance.onRobotDisconnected (this);

		// doorgeven aan de robotmanager dat we gedisconnect zijn
		BLERobotManager.Instance.robotDisconnected (this);

		// terug scannen voor nieuwe peripherals:
		BLERobotManager.Instance.ScanForPeripherals ();
	}
	
	public void DiscoveredServiceAction(string id, string serviceUUID)
	{
		Debug.Log ("BLE - " + this.id + " : discovered service uuid '" + serviceUUID + "'");
	}
	
	public void DiscoveredCharacteristicAction(string id, string serviceUUID, string discoveredCharacteristicUUID)
	{
		Debug.Log ("BLE - " + this.id + " : discovered characteristic uuid '" + characteristicUUID + "'");

		if(discoveredCharacteristicUUID == this.characteristicUUID)
		{
			Debug.Log ("BLE - " + this.id + " : subscribing to notifications");
			BluetoothLEHardwareInterface.SubscribeCharacteristic (this.id, serviceUUID, discoveredCharacteristicUUID, DidUpdateNotifiationStateForCharacteristicAction, DidUpdateCharacteristicValueAction);
		

			GameManager.Instance.onRobotConnected(this);
		}
	}

	public void DidUpdateNotifiationStateForCharacteristicAction(string characteristicUUID)
	{
		Debug.Log ("BLE - " + this.id + " : DidUpdateNotifiationStateForCharacteristicAction()"); // geen idee wanneer da voorkomt
	}
	
	public void DidUpdateCharacteristicValueAction(string characteristicUUID, byte[] data)
	{

		for (int i = 0; i<data.Length; i++) {
			switch(data[i]) {
			case 0x02:
				receivingCommandStarted = true;
				receivingCommandbytes.Clear();
				break;

			case 0x03:
				if(receivingCommandStarted) {
					parseCommand(receivingCommandbytes.ToArray());
					receivingCommandStarted = false;
				}
				break;

			default:
				if(receivingCommandStarted){
					receivingCommandbytes.Add(data[i]);
				}
				break;
			}
		}
	}

	public void parseCommand(byte[] command) {
		string commandString = Encoding.UTF8.GetString(command, 0, command.Length);
		Debug.Log("BLE - " + this.id + " : incomming command " + commandString);

		string commandWithoutFirstLetter = commandString.Substring (1);
		string firstLetter = commandString.Substring (0, 1);


		switch (firstLetter) {
		case "S":
			incommingLineSensor(commandWithoutFirstLetter);
			break;
				
		case "P":
			incommingPingSensor(commandWithoutFirstLetter);
			break;
					
		case "R":
			incommingRSSI(commandWithoutFirstLetter);
			break;
		}
	}

	public void incommingLineSensor(string data)
	{
		Debug.Log("BLE - " + this.id + " : incomming line sensor data: " + data);
	}
	
	public void incommingPingSensor(string data)
	{
		Debug.Log("BLE - " + this.id + " : incomming ping sensor data: " + data);
	}
	
	public void incommingRSSI(string data)
	{
		try
		{
			this.rssi = Convert.ToInt32(data);
			
			float normalizedRssi = ((float)rssi + 255) / 255;
			
			Debug.Log("BLE - " + this.id + " : incomming rssi: " + rssi + "| normalized value: " + normalizedRssi);

			GameManager.Instance.onRSSIUpdate(this, rssi);
		}
		catch (FormatException)
		{
			Debug.Log("BLE - " + this.id + " : rssi value is no Int32");
		}
	}

}


