# How to get project to your computer
1. Log in to GitHub
2. Clone this project with VSCode:[link](https://github.com/anhphan21/final_project)
3. In VSCode, install extension GitHub Pull Requests [ext link](https://marketplace.visualstudio.com/items?itemName=GitHub.vscode-pull-request-github)
4. Done you can work on your own !

# How to submit your change to GitHub
In VSCode
0. If this is the first time you submit the code, then create the new branch
   1. Create new branch with your name
   2. ctrl+shift+p, then type new branch
   3. enter your name
   4. Done !!!
   1. Go to `source control` or ctrl+shift+g
      1. Changes: What you have modified in project (D for delete, M for modified)
      2. Type some comment on what you have changed in the code in message box !
      3. Commit for adding the code, Push to push the code to your branch on Github!
      4. If you done with your code and want to merge your branch with main branch, create pull request !
      5. After request is accepted, then do these steps in Git terminal
      
      ```
         git checkout master    //go to the master branch
         git pull origin master //Update the code in master branch
         git checkout <branch_name> //Go back to your branch
         git merge master       // Merge your code with the master
         //If there are some conficts (differences) in your code with the master code, then you have to resolve by manually
         git add [file1] [file2] ... 
         git commit -m "Resolve merge conflicts between master and <your_branch_name>"
         git push origin test
      ```
   