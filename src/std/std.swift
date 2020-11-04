class w_Arr<T>{
  var data : Array<T> = [];
  init(data:[T]){self.data = data;}
  init(zero:T,n:Int){self.data = Array<T>(repeating:zero,count:n);}
  func remove(i:Int,n:Int){self.data.removeSubrange(i..<(i+n));}
  func slice(i:Int,n:Int)->w_Arr<T>{
    return w_Arr<T>(data:Array(self.data[i..<(i+n)]));
  }
}
class w_Map<K:Hashable,V>{
  var data : Dictionary<K,V> = [:];
}
static func w_charCode(x:String) -> Int{
  return (Int)(x.unicodeScalars[x.unicodeScalars.startIndex].value);
}
static func w_strSlice(x:String,i:Int,n:Int)->String{
  return String(Array(x)[i..<(i+n)]);
}
static func w_INT(x:Bool)->Int{return x ? 1 : 0;}
static func w_BOOL(x:Int)->Bool{return x != 0;}
static func w_NOT(x:Int)->Int{return (x == 0) ? 1 : 0;}
static func w_strBang(x:String?)->String{return x!;}