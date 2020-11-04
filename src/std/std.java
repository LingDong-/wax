public static <T> void w_arrRemove(ArrayList<T> a, int i, int n){
  a.subList(i,i+n).clear();
}
public static <T> ArrayList<T> w_arrSlice(ArrayList<T> a, int i, int n){
  return new ArrayList<T>(a.subList(i,i+n));
}
public static String w_strSlice(String a, int i, int n){
  return a.substring(i,i+n);
}
public static <K,V> V w_mapGet(HashMap<K,V> map, K key, V defaultValue) {
	// == java 8 getOrDefault
    V v;
    return (((v = map.get(key)) != null) || map.containsKey(key))
        ? v
        : defaultValue;
}
public static boolean w_BOOL(int x){return x!=0;}
public static int w_INT(boolean x){return x?1:0;}
public static int w_NOT(int x){return (x==0)?1:0;}
