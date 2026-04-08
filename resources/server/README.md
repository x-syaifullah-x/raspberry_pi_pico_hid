#
#
### RUN
```sh
su -c "go run -ldflags='-s -w' -trimpath src/main.go /dev/hidraw0"
```

### BUILD
```sh
go build -ldflags="-s -w" -trimpath -o build/raspbarry_pi_pico_hid_server src/main.go
```