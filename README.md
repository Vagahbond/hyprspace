# hyprspace

The most useful hyprland plugin ever created on past and future history.

I learnt to do some graphic programming specifically for this occasion.

This plugin exists for the sole purpose of crushing the competition on a ricing competition themed with space.

https://github.com/Vagahbond/hyprspace/assets/56230065/ba61c0d0-2a91-48a0-afe0-41398caaeca4



Performace is incredibly bad(although it displays fluidly on my screen) and the codebase is an absolute joke.

things left to do:

- [ ] Calculate the real start witdth using `coeff`: it needs to use the normal vector of the trail to calculate the witdth it's supposed to have depending on the trail's orientation
- [ ] Implement a function that'll do beter than Vaxry's at imitating a comet's trail `F(t) = base_width * 1 - (t/total_time)`.
- [ ] Add stars shader on inactive winows. I need it to be TACHY.
- [ ] Change the config attributes from `hyprtrails` to `hyprspace` and chose what should be parameterized.

Only setting you may want to change(for now):

```ini
plugin {
    hyprtrails {
        color = rgba(ffaa00ff)
    }
}

```

I started from Vaxry's [hyprtrails plugin](https://github.com/hyprwm/hyprland-plugins/tree/main/hyprtrails) which I modified.
Special thanks to him for taking a bit of time to explain to me how his code works !
