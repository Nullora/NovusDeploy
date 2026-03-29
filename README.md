# NovusDeploy
A tool to help you keep your code up-to-date across multiple files.
# Installation (Linux Only)
## Clone the repo (MANDATORY)
```bash
git clone https://github.com/Nullora/NovusDeploy
```
## Setup folders (MANDATORY)
```bash
make setup # makes neccecary folders and files
```
## Build your own binary (OPTIONAL)
If you are gonna make changes or do not trust my binary, do this:
```bash
make clean
make # requires gcc installed
```
## Copy to path (MANDATORY)
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
ndep add-d ~/destination/folder <yourtag>
```
To deploy the new source file to all destination files
```bash
ndep dep <yourtag>
```
To backup your source file
```bash
ndep set <yourtag>
```
To revert your source file to last backup
```bash
ndep rev <yourtag>
```
## Multi tag / Tag groups (Advanced)
To make a new group
```bash
ndep addg <groupname>
```
This will prompt you for a list of already existing tags:
```bash
Tag to add: <yourtag1> # must be an existing tag
Tag to add: <yourtag2> # must be an existing tag
Tag to add: hlt # ends tag group
```

To set a backup for that group
```bash
ndep setg <groupname>
```
To revert group to previous backup
```bash
ndep revg <groupname> # this reverts the whole group, use with caution
```

To deploy all tags in a group to their respective destination folders
```bash
ndep depg <groupname> # this deploys every tag to its destination
```