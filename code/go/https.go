package main

import (
    "fmt"
    "net/http"
    "strings"
    "log"
)

func sayhello(w http.ResponseWriter, r *http.Request) {
    r.ParseForm()
    fmt.Println(r.Form)
    fmt.Println("path", r.URL.Path)
    fmt.Println("scheme", r.URL.Scheme)
    fmt.Println(r.Form["url_long"])
    for k, v := range r.Form {
        fmt.Println("key:", k)
        fmt.Println("val:", strings.Join(v, ""))
    }
    fmt.Fprintf(w, "Hello world!")
}

/**
openssl genrsa -out ca-key.pem 2048
openssl req -new -key ca-key.pem -out ca-cert.csr
openssl req -new -x509 -key ca-key.pem -out ca-cert.pem -days 1095
*/
func main() {
    http.HandleFunc("/", sayhello)
    err := http.ListenAndServeTLS(":443", "ca-cert.pem", "ca-key.pem", nil)
    if err != nil {
        log.Fatal("ListenAndServe: ", err)
    }
}
