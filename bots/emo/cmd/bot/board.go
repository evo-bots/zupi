package main

import (
	"fmt"
	"os"
	"os/signal"

	"github.com/evo-bots/zupi/bots/emo/robot"
	"github.com/robotalks/mqhub.go/mqtt"
)

type boardCmd struct {
	ID         string
	Host       string `n:"mqtt-host"`
	Port       int    `n:"mqtt-port"`
	Namespace  string `n:"mqtt-ns"`
	SerialPort string `n:"serial-port"`
	VideoDev   string `n:"video-dev"`
	VideoW     int    `n:"video-width"`
	VideoH     int    `n:"video-height"`
}

func (c *boardCmd) Execute(args []string) (err error) {
	boardOpts := robot.NewOptions()
	boardOpts.FirmataDevice = c.SerialPort
	boardOpts.VideoCapDevice = c.VideoDev
	boardOpts.VideoCapWidth = c.VideoW
	boardOpts.VideoCapHeight = c.VideoH
	board := robot.NewBoard(boardOpts)
	opts := mqtt.NewOptions()
	opts.AddServer(fmt.Sprintf("tcp://%s:%d", c.Host, c.Port))
	opts.Namespace = c.Namespace
	opts.ClientID = c.ID
	conn := mqtt.NewConnector(opts)
	if err = conn.Connect().Wait(); err != nil {
		return
	}
	if _, err = conn.Publish(board); err != nil {
		return
	}

	if err = board.Start(); err != nil {
		return
	}

	sigCh := make(chan os.Signal)
	signal.Notify(sigCh, os.Interrupt)
	<-sigCh

	board.Stop()
	return
}
