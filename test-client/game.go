package main

import (
	"time"

	"github.com/TypicalAM/boomberman/test-client/pb"
)

var FuseTime = time.Second * 3

type Coord struct {
	x, y float32
}

type Entity interface {
	Name() string
	Coords() Coord
}

type Player struct {
	name   string
	coords Coord
	color  pb.PlayerColor
}

func (p Player) Name() string {
	return p.name
}

func (p Player) Coords() Coord {
	return p.coords
}

type Bomb struct {
	name      string
	coords    Coord
	timestart int64
}

func (b Bomb) Name() string {
	return b.name
}

func (b Bomb) Coords() Coord {
	return b.coords
}

func (b Bomb) ShouldExplode() bool {
	return time.Now().Add(FuseTime).After(time.Unix(b.timestart, 0))
}

// Check who shold be in the blast radius, returns a list of ints
func (b Bomb) CheckRadius(entities []Entity) []int {
	result := make([]int, 0, len(entities))
	for i, entity := range entities {
		coords := entity.Coords()
		if ((b.coords.x-3) <= coords.x && coords.x <= (b.coords.x+3)) && ((b.coords.y-3) <= coords.y && coords.y <= (b.coords.y+3)) {
			result = append(result, i)
		}
	}
	return result
}
