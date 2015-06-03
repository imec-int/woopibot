using System;
using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;

//MANAGER
public class GameManager : Singleton<GameManager>
{
	//FIELDS
	Image scanner;
	
	public event Action<string, int> OnCardScanned; 
	
	//PROPERTIES

	
	//METHODS
	void Start()
	{
		
	}

	public void Init(Image scanner) // aangeroepen van SetupSceneScript
	{
		this.scanner = scanner;
	}

	public void onRobotConnected(BLERobot robot)
	{
		Debug.Log ("onRobotConnected("+robot.id+")");
	}

	public void onRobotDisconnected(BLERobot robot)
	{
		Debug.Log ("onRobotDisconnected("+robot.id+")");

		if (scanner)
			scanner.sprite = Resources.Load<Sprite>("Sprites/dist_5");
	}
	
	public void onRSSIUpdate(BLERobot robot, int rssi)
	{
		Debug.Log ("onRSSIUpdate("+robot.id+","+rssi+")");


		if (!scanner)
			return;


		if (rssi <= -80) {
			scanner.sprite = Resources.Load<Sprite>("Sprites/dist_5");
		}

		if (-80 < rssi && rssi <= -70) {
			scanner.sprite = Resources.Load<Sprite>("Sprites/dist_4");
		}

		if (-70 < rssi && rssi <= -65) {
			scanner.sprite = Resources.Load<Sprite>("Sprites/dist_3");
		}

		if (-65 < rssi && rssi <= -60) {
			scanner.sprite = Resources.Load<Sprite>("Sprites/dist_2");
		}

		if (-60 < rssi) {
			scanner.sprite = Resources.Load<Sprite>("Sprites/dist_1");
		}
	}

}
