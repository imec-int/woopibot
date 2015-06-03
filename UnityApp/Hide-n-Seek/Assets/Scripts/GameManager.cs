using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;

//MANAGER
public class GameManager : Singleton<GameManager>
{
	//FIELDS
	
	public event Action<string, int> OnCardScanned; 
	
	//PROPERTIES

	
	//METHODS
	void Start()
	{

	}
	
	public void Update()
	{

	}
	

	
	public void onNewBracelet(String braceletId)
	{
		Debug.Log (braceletId);
	}

	public void ScannedCard(string playerID, int cardID)
	{

	}
}
