
bool b1;
bool* b2;
bool[5] b3;
bool*[5] b4;
bool**[5] b5;

enum Test
{
  PROVA,
  PROVA2 = 5,
  PROVA3,
  PROVA4,
  PROVA5 = 10,
  PROVA6,
  PROVA7 = 25
};


Test testVar;
Test*[10] asdiu;

byte[] array = {1,2,3,4};

function void antani(byte x, byte y, Test asda)
{

}

byte[] antani = "foobar";
byte[5] array;
word[8] antanius;



function word fibonacci(word value)
{
  word aaaa = 10;
  
  if ((value == 1) | (value == 2))
    return 1;
  else
    return fibonacci(value - 1) + fibonacci(value - 2);
}

function void main()
{
  Test asd;
  
  byte x = PROVA3;
  
  {
    word sblindi = 10;
  }
  
  print(fibonacci(5));
  x = 2 + y;
  
  while (x > 0)
    x = x - 1;
    
  if (x > 5)
    return test;
  elseif (x < 5 || y < 10)
    return PROVA5 + PROVA7 + 1;
  elseif (y == x)
    return antani;
  else
    return 0;
    
  //return (value == 1 | value == 2) ? 4 : 0;
}