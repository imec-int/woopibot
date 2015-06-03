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
		int something = 0;

		switch (something)
		{
		case 1:
			scanner.sprite = Resources.Load<Sprite>("Sprites/avatar_big");
			break;
		case 2:
			scanner.sprite = Resources.Load<Sprite>("Sprites/avatar_big2");
			break;
		case 3:
			scanner.sprite = Resources.Load<Sprite>("Sprites/avatar_big3");
			break;
		}
	}
	


	public void proximityValues(string data)
	{
		Debug.Log(data);
	}
}
