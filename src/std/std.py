def w_arr_remove(a,i,n):del a[i:i+n]
def w_slice(a,i,n):return a[i:i+n]
def w_map_get(m,k,d):
    try:return m[k]
    except:return d
def w_map_remove(m,k):
    try:del m[k]
    except:pass