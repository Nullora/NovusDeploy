# NovusDeploy
A tool to help you keep your code up-to-date across multiple files.
# Installation (Linux Only)
## Clone the repo
```bash
git clone https://github.com/Nullora/NovusDeploy
```
## Setup folders
```bash
make setup # makes necessary folders and files
```
## Build your own binary
```bash
make clean
make # requires gcc installed
```
## Copy to path
Copy it to PATH
```bash
make deploy
```
# Usage
## Single tags (Beginner)
To add a source file and assign it to a tag
```bash
ndep add ~/path/to/file <yourtag>
```
To add a destination file and assign it to a tag
```bash
ndep add-d ~/destination/folder <yourtag> # "add destination"
```
To delete a tag
```bash
ndep del <yourtag> # 'delete'
```
To deploy the new source file to all destination files
```bash
ndep dep <yourtag> # "deploy"
```
To backup your source file
```bash
ndep set <yourtag>
```
To revert your source file to last backup
```bash
ndep rev <yourtag> # "revert"
```
To directly deploy your tag to PATH instead of manually adding PATH to your dests
```bash
ndep depp <yourtag> # "deploy path"
```
## Multi tag / Tag groups (Advanced)
To make a new group
```bash
ndep addg <groupname> # "add group"
```
This will prompt you for a list of already existing tags:
```bash
Tag to add: <yourtag1> # must be an existing tag
Tag to add: <yourtag2> # must be an existing tag
Tag to add: hlt # ends tag group
```

To set a backup for that group
```bash
ndep setg <groupname> # "set group"
```
To revert group to previous backup
```bash
ndep revg <groupname> # this reverts the whole group, use with caution
```

To deploy all tags in a group to their respective destination folders
```bash
ndep depg <groupname> # this deploys every tag to its corresponding destination
```