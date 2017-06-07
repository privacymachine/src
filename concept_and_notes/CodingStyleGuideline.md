

# Coding Style Guide

General:
Writing **Readable Code** is the most important rule.  
You will love it when you read your code from last year ;)  
If it's easy readable it is easy comprehensible which is also an security benefit!

## Additional brackets are welcome:
```c++
if ( (!hasErrors && Foo.success()) || errCount == 0 ) ...
```

## CamelCase-Names
Only the first letter has to be upper case:  

**allowed: EuStates**  

**not allowed: EUStates**

### Class-Names Upper case, functions and members lower case.

## private members
private members needs an underline as suffix i.e: internalData_

## brackets: curly brackets deserve a new line
```c++
if (true)
{
}
```
## When brackets are used:
**allowed:**  
```c++
if (i==3) return true;
```
  
**not allowed:**  
```c++
// not clear if this comment line described the code till the return 
// or continues to "i = 2"
if (true)
  return false;

int i = 2;
```

**recommended:**  
```c++
// comment includes line "i = 2" 
if (true)
{
  return false;
}
int i = 2;
```
### Class, Structs, Switches: all "{" deservers a new line

### Members of Structs start with Uppercase
```c++
struct UpdateInfoBinary
{
  PmVersion Version;
  QString Title;
  QString Description;
  QDateTime Date;
  QList<CheckSumListBinary> CheckSums;
};
```

## Spaces, Tabs, Line Length
required: use 2 spaces instead of tabs  
Maximum line length is 150 characters.

**additional spaces are optional, if readability is encouraged:**  
```c++
if ( !json_file.open(QIODevice::ReadOnly | QIODevice::Text) )
```
  
**booth allowed:**  
```c++
if (bOk == true) return false;
if(bOk == true) return false;
```
  
**required after parameter separator:** 
```c++
printf("foo: %d %d", 13, 12);
```

**not allowed:**  
```c++
printf("foo: %d %d",13,12);
```

**not allowed:**  
```c++
Class . function();
Class -> function();
```

## Required: Use nullptr instead of 0 for pointers

**not allowed:**  
```c++
if(!PointerToObj)
```

**not optimal:**  
```c++
if(PointerToObj != 0)
```

```c++
if(PointerToObj != NULL)
```

**best way:**  
```c++
if(PointerToObj != nullptr)
```

## Required: Use c++11 features over QT features

### for each

**not allowed:**
```c++
foreach (var, container) 
{
  var++;
}
```
  
**allowed:**
```c++
for (var : container) 
{
  var++;
}
```

