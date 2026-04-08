package main

import (
	"fmt"
	"strings"
)

type HIDItem struct {
	Tag  byte
	Type byte
	Size byte
	Data []byte
}

// Tag types
const (
	TypeMain   = 0x00
	TypeGlobal = 0x01
	TypeLocal  = 0x02
)

// Global tags
var globalTags = map[byte]string{
	0x0: "Usage Page",
	0x1: "Logical Minimum",
	0x2: "Logical Maximum",
	0x3: "Physical Minimum",
	0x4: "Physical Maximum",
	0x5: "Unit Exponent",
	0x6: "Unit",
	0x7: "Report Size",
	0x8: "Report ID",
	0x9: "Report Count",
}

// Main tags
var mainTags = map[byte]string{
	0x8: "Input",
	0x9: "Output",
	0xA: "Collection",
	0xB: "Feature",
	0xC: "End Collection",
}

// Local tags
var localTags = map[byte]string{
	0x0: "Usage",
	0x1: "Usage Minimum",
	0x2: "Usage Maximum",
}

func ParseDescriptor(desc []byte) []HIDItem {
	var items []HIDItem
	i := 0
	for i < len(desc) {
		b := desc[i]
		size := b & 0x03
		typ := (b >> 2) & 0x03
		tag := (b >> 4) & 0x0F

		if size == 3 {
			size = 4
		}

		data := desc[i+1 : i+1+int(size)]
		items = append(items, HIDItem{Tag: tag, Type: typ, Size: size, Data: data})
		i += 1 + int(size)
	}
	return items
}

func printParsed(items []HIDItem) {
	indent := 0
	for _, item := range items {
		var name string
		switch item.Type {
		case TypeMain:
			name = mainTags[item.Tag]
			if item.Tag == 0xC { // End Collection
				indent--
			}
		case TypeGlobal:
			name = globalTags[item.Tag]
		case TypeLocal:
			name = localTags[item.Tag]
		}
		if name == "" {
			name = fmt.Sprintf("Unknown(type=%X,tag=%X)", item.Type, item.Tag)
		}

		// Hitung nilai
		val := 0
		for j, b := range item.Data {
			val |= int(b) << (j * 8)
		}

		fmt.Printf("%s%s: 0x%X (%d)\n",
			spaces(indent*2), name, val, val)

		if item.Type == TypeMain && item.Tag == 0xA { // Collection
			indent++
		}
	}
}

func spaces(n int) string {
	var s strings.Builder
	for range n {
		s.WriteString("  ")
	}
	return s.String()
}

func GetReportCount(items []HIDItem) int {
	for _, item := range items {
		// Type = Global (0x01), Tag = 0x09 = Report Count
		if item.Type == 0x01 && item.Tag == 0x09 {
			val := 0
			for j, b := range item.Data {
				val |= int(b) << (j * 8)
			}
			return val
		}
	}
	return 64
}
