# SOHL ported to halflife-updated

This code ports Spirit to the halflife-updated repo, bugs and all.
No fixes to ANY version of Spirit will be done, any bugs that exist in SOHL
will still exist in these branches. The only fixes that will happen are the
ones that were a side effect of SOHL being based on an older version of the
Half-Life SDK.

Any accidental problems caused by mistakes during the merge from SOHL into
halflife-updated will be fixed if an issue/PR is created to let me know.

The FGD changes are from ones I've found online using Google, they may or
may not be correct, I haven't verified them.

Differences between the original SOHL versions are listed below.

## 1.0

Render FX #21 was added to the engine in Steam as the "light multiplier"
effect, so the "reflection" render fx is now #22.

In the FGD, find `@BaseClass = RenderFxChoices` and scroll down to this
line:

```c
    21: "Reflection (Models)"
```

Change it to:

```c
    22: "Reflection (Models)"
```

If you are unable to do this and need complete backwards compatibility
with SOHL, comment out the line containing `kRenderFxLightMultiplier`
in `const.h` to get it working again. However, this may have unexpected
side effects due to the engine thinking that this is the "light multiplier"
effect (though I'm not sure what this effect does or if it works).

## 1.4

Similar to above, `kRenderFxEntInPVS` is render fx 23 instead of 22.