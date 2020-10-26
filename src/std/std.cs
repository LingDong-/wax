public static V w_mapGet<K,V>(Dictionary<K,V> map, K key, V defaultValue) {
	V v;
	if (map.TryGetValue(key,out v)){
		return v;
	}
	return defaultValue;
}
public static bool w_BOOL(int x){return x!=0;}
public static int w_INT(bool x){return x?1:0;}
public static int w_NOT(int x){return (x==0)?1:0;}
public static int w_AND(int x, int y){return ((x!=0) && (y!=0))?1:0;}
public static int w_OR(int x, int y){return ((x!=0) || (y!=0))?1:0;}
