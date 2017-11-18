# nsnakepp
snake - ncurses implementation

It's a pretty simple snake implementation, if you want you can personalize colors and game complexity you can do it editing *nsnake.cpp*, nb. *nsnake.cpp* is only a fronted for *SnakeEngine.cpp*.

###### On RPI (raspbian)
``` bash
sudo apt install libssl-dev libncursesw5-dev -y
git clone https://github.com/Manu-sh/nsnakepp && make -C nsnakepp
```
###### On Archlinux
``` bash
sudo pacman -S ncurses openssl
git clone https://github.com/Manu-sh/nsnakepp && make -C nsnakepp
```

<img src="https://raw.githubusercontent.com/Manu-sh/nsnakepp/master/screenshots/01.jpg" width="450" height="250"/>


###### Copyright © 2017, [Manu-sh](https://github.com/Manu-sh), s3gmentationfault@gmail.com. Released under the [GPL3 license](LICENSE).
