using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.UI;

public class SetupSceneScript : MonoBehaviour 
{
	//Display players
	public Transform canvas;
	public Image scanner;


	void Start()
	{
		BLERobotManager.Instance.StartBLE ();
		GameManager.Instance.Init (scanner);
	}
}
