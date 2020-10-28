function w_trunc(x)         if x < 0 then return math.ceil(x) else return math.floor(x) end end
function w_vec_init(z,n)    local x = {} for i=1,n do x[i]=z end return x end
function w_arr_slice(x,i,n) local y = {} for j=(i+1),(i+n) do y[j-i]=x[j] end return y end
function w_arr_remove(x,i,n)for j=(i+n),(i+1),-1 do table.remove(x,j) end end
function w_map_len(x)       local cnt = 0 for k,v in pairs(x) do cnt = cnt + 1 end return cnt end
function w_strcpy(x,i,n)    return string.sub(x,i+1,i+n) end