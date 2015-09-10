package config

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os"
)

/* config.json

{
  "run_mode": "dev",
  "app_name": "palette",
  "app_address": "10.141.123.243",
  "http_port": 443
}

*/

type Configuration struct {
	Run_mode string
	App_name string
	App_address string
	Http_port int
}

var Cfg Configuration

func LoadConfig () {
    fmt.Printf("LoadConfig\n");
    file, err := os.OpenFile("config/config.json",  os.O_RDWR, 0666)
    if err != nil {
        fmt.Printf("open failed!\n")
        return
    }
    defer file.Close()
    
    dec := json.NewDecoder(file)
    for {
	if err := dec.Decode(&Cfg); err == io.EOF {
	    break
	} else if err != nil {
            log.Fatal(err)
	}
//	fmt.Printf("%s, %s\n", Cfg.Run_mode, Cfg.App_name)
    }
}
