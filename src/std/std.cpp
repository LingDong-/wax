template <typename T> 
inline void w_arr_insert (std::vector<T>* arr, int i, T x){
  arr->insert(arr->begin()+i,x);
}

template <typename T>
inline void w_arr_remove (std::vector<T>* arr, int i, int n){
  arr->erase(arr->begin()+i,arr->begin()+i+n);
}

template <typename T>
inline std::vector<T>* w_arr_slice (std::vector<T>* arr, int i, int n){
  return new std::vector<T>(arr->begin()+i,arr->begin()+i+n);
}

template <typename K, typename V>
inline V w_map_get (std::map<K,V>* m, K k, V defau){
  typename std::map<K,V>::iterator it = m->find(k);
  if (it != m->end()){
    return it->second;
  }
  return defau;
}

template <typename T, std::size_t N>
inline std::array<T,N>* w_vec_init (T v){
	std::array<T,N>* vec = new std::array<T,N>;
	vec->fill(v);
	return vec;
}
