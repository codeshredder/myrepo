package main
import "io/ioutil"
func listAll(path string) {
    files, _ := ioutil.ReadDir(path)
    for _, fi := range files {
        if fi.IsDir() {
        //listAll(path + "/" + fi.Name())
        println(path + "/" + fi.Name())
    } else {
        println(path + "/" + fi.Name())
    }
    }
}
func main(){
    listAll(".")
}
