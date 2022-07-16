function word fibonacci(word v)
{
  if (v == 0 || v == 1)
    return 1;
  elseif
    return fibonacci(v - 1) + fibonacci(v - 2);
}

function void main()
{
  word result = fibonacci(5);
}