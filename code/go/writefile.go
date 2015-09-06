package main

import (
    "fmt"
    "os"
)

func main() {
    f, err := os.OpenFile("test.txt",  os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0666)
    defer f.Close()

    if err != nil {
        fmt.Printf("open failed!\n")
        return
    }

    fmt.Fprintf(f, "Hello World!")
    fmt.Printf("OK")
    return
}
