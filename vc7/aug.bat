call "%VS71COMNTOOLS%vsvars32.bat"
devenv.com aug.sln /rebuild Debug /project setup
devenv.com aug.sln /rebuild Release /project setup
