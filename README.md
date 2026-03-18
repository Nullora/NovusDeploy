# NovusDeploy
Hhardcoded to fit my computer and made for personal usage to help me keep some files updated across all my local projects so I don't manually recopy it.
Check out [NovusNet](https://github.com/Nullora/NovusNet) and star it for more future updates!
# Why I made it
I made NovusDeploy to help me keep my projects updated with the latest NovusNet patch fully locally.
Say you have a library you made. You made projects with that library and now depend on it to stay up to date. Upon any new library update, you need to manually copy the new library files and paste them in all your projects. Over time, this gets super repetitive. NovusDeploy does this for you, just add a source file with **ndep add filepath tag** and add your project files with **ndep add-d filepath tag**. Upon any new update to your library, run **ndep dep tag** and watch that file be synced across all your projects!
# WARNING
You can't add more than 1 source file in 1 tag. Trying will overwrite the old source file:
- **ndep add file1 tag1**
- **ndep add file2 tag1**
this will wipe file1 from ndep memory and only sync file2 to every destination