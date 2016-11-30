package main

import (
	"github.com/codingbrain/clix.go/flag"
)

var cliDef *flag.CliDef

func init() {
	cliDef = &flag.CliDef{
		Cli: &flag.Command{
			Name: "bot",
			Desc: "Robot",
			Commands: []*flag.Command{
				&flag.Command{
					Name: "board",
					Desc: "Run robot board",
					Options: []*flag.Option{
						&flag.Option{
							Name:  "id",
							Alias: []string{"i"},
							Desc:  "Board Unique ID",
						},
						&flag.Option{
							Name:    "mqtt-host",
							Alias:   []string{"h"},
							Desc:    "MQTT Host",
							Default: "127.0.0.1",
						},
						&flag.Option{
							Name:    "mqtt-port",
							Alias:   []string{"p"},
							Desc:    "MQTT Port",
							Type:    "int",
							Default: 1883,
						},
						&flag.Option{
							Name:    "mqtt-ns",
							Alias:   []string{"t"},
							Desc:    "MQTT topic prefix (aka namespace)",
							Default: "zupi/bots/emo",
						},
						&flag.Option{
							Name:    "serial-port",
							Desc:    "Serial Port for Firmata",
							Default: "/dev/ttyAMA0",
						},
						&flag.Option{
							Name:    "video-dev",
							Desc:    "Video Capture Device",
							Default: "/dev/video0",
						},
						&flag.Option{
							Name:    "video-width",
							Desc:    "Video Width",
							Type:    "int",
							Default: 640,
						},
						&flag.Option{
							Name:    "video-height",
							Desc:    "Video Height",
							Type:    "int",
							Default: 480,
						},
					},
				},
				&flag.Command{
					Name:  "version",
					Alias: []string{"ver"},
					Desc:  "Show version",
				},
			},
		},
	}
	cliDef.Normalize()
}
