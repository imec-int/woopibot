/*
 * Ter info: BluetoothLEHardwareInterface.Log (str); logt naar de iOS output window onafhankelijk van het feit of "Script Debugging" aanstaat of niet
 * 
 * */

using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class BLERobotManager : Singleton<BLERobotManager>
{
	string allowedName = "woopibot";
	string serviceUUID = "2220";


	private List<BLERobot> bleRobots = new List<BLERobot>();
	

	public void StartBLE() // aangeroepen van SetupSceneScript
	{
		Debug.Log("BLE: initalizing");
		BluetoothLEHardwareInterface.Initialize (true, false, InitializedAction, ErrorAction);
	}

	// Callbacks (Actions/Delegates):
	public void InitializedAction ()
	{
		Debug.Log("BLE: initialized");
		ScanForPeripherals ();
	}
	
	public void ErrorAction (string error)
	{
		// TODO: show error in UI
		Debug.Log("BLE: initialization error: " + error);
	}


	public void ScanForPeripherals()
	{
		Debug.Log("BLE: scanning for peripherals with service uuid '" + serviceUUID + "'");
		BluetoothLEHardwareInterface.ScanForPeripheralsWithServices (new string[] { serviceUUID }, DiscoveredPeripheralAction);
	}

	public void DiscoveredPeripheralAction(string identifier, string name)
	{
		Debug.Log("BLE - " + identifier + " : discovered peripheral with name '" + name + "'");
		if(name != allowedName) Debug.Log("BLE - " + identifier + " : not connecting");

		// maak een nieuw robot objectje aan:
		BLERobot robot = new BLERobot (name, identifier);

		// connect en doe verder met BLE stuff in het objectje:
		robot.Connect ();
	}


	public void robotDisconnected(BLERobot robot)
	{
		if(bleRobots.Contains(robot))
			bleRobots.Remove(robot);

		robot = null;
	}



}


