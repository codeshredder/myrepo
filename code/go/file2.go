package main

import "io/ioutil"
import "os"
import "fmt"

func listAll(path string) {
    files, _ := ioutil.ReadDir(path)
    for _, fi := range files {
        if fi.IsDir() {
            println("into:" + path + "/" + fi.Name())
            listAll(path + "/" + fi.Name())
        } else {
            println(path + fi.Name())
        }
    }
}
func main(){

    fmt.Printf("args:[%d][%s][%s]\n", len(os.Args), os.Args[0], os.Args[1])

    if len(os.Args) > 1 {
        listAll(os.Args[1])
    }else {
	listAll(".")
    }
}
