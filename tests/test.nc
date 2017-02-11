struct Point
{
  word x;
  byte y;
};

struct ColoredPoint
{
  Point point;
  byte color;
};

Point[10] points;
Foo[5] asda;

function word fibonacci(word value)
{
  //byte value;
  
  if ((value == 1) | (value == 2))
    return 1;
  else
    return fibonacci(value - 1) + fibonacci(value - 2);
}

function void main()
{
  Point point;
  Point* pts;
  
  asda[3] = 10;

  fibonacci(10);
}