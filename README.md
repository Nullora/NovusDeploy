# NovusDeploy
Also hardcoded to fit my computer and made for personal usage to help me keep NovusNet updated across all my local projects so I don't manually recopy it.
Check out [NovusNet](https://github.com/Nullora/NovusNet) and star it for more future updates!
# Why I made it
I made NovusDeploy to help me keep my projects updated with the latest NovusNet patch fully locally.
Say you have a library you made. You made projects with that library and now depend on it to stay up to date. Upon any new library update, you need to manually copy the new library files and paste them in all your projects. Over time, this gets super repetitive. NovusDeploy does this for you, just add a source file with **ndep add filepath tag** and add your project files with **ndep add-d filepath tag**. Upon any new update to your library, run **ndep deploy tag** and watch that file be synced across all your projects!