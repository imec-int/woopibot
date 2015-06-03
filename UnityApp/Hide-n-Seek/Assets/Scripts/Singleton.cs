using UnityEngine;

/*Singleton is the template class that we use for managers*/
public class Singleton<T> : MonoBehaviour where T : MonoBehaviour
{
	//FIELDS
	private static T _instance;
	private static object _lock = new object();
	private static bool _applicationIsQuitting;
	
	public static T Instance
	{
		get
		{
			if (_applicationIsQuitting)
			{
				Debug.LogWarning("[Singleton] Instance" + typeof(T).ToString() + "has already been destroyed" +
				                 " because the application is being terminated.");
			}
			
			lock (_lock)
			{
				if (_instance == null)
				{
					_instance = (T)FindObjectOfType(typeof(T));
					
					if (_instance != null && FindObjectsOfType(typeof(T)).Length > 1)
					{
						Debug.LogError("[Singleton] Something when really wrong. More instances of an object " + typeof(T).ToString() +
						               " exist in your scene!");
						return _instance;
					}
					
					if (_instance == null)
					{
						GameObject singleton = new GameObject();
						_instance = singleton.AddComponent<T>();
						singleton.name = typeof(T).ToString();
						
						//make sure we can reuse our singleton of this object
						DontDestroyOnLoad(singleton);
					}
				}
				
				return _instance;
			}
		}
	}
	
	public virtual void OnDestroy()
	{
		//Unity can destroy objects in random order. If we start on our destroy,
		//we won't let other objects create another instance of this object. If we 
		//would allow it a ghost object would be generated that stays alive in the scene
		_applicationIsQuitting = true;
	}
}
