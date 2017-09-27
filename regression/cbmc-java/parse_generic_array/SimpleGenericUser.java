interface BasicInterface
{
  int getX();
}

class Foo implements BasicInterface
{
  public int x;

  public int getX() {
    return x;
  }
}

class Bar extends Foo
{}

class GenericArray<T>
{
  public T [] t;
}

public class SimpleGenericUser
{
    // Test a wild card generic type
  public static void processSimpleGeneric(GenericArray<?> x) {
    assert(x.t == null);
  }

  // Test a wildcard generic bound by an interface
  public static void processUpperBoundInterfaceGeneric(GenericArray<? extends BasicInterface> x) {
    assert(x.t.length >= 1 && x.t[0] != null && x.t[0].getX() == 4);
  }

  // Test a wild card generic bound by a class
  public static void processUpperBoundClassGeneric(GenericArray<? extends Foo> x) {
    assert(x.t.length >= 1 && x.t[0] != null && x.t[0].getX() == 4);
  }

  // It isn't legal to have an wild card with two upper bounds
  // Per language spec on intersection types

  public static void processLowerBoundGeneric(GenericArray<? super Foo> x, Foo assign) {
    assert(x.t.length >= 1);
    x.t[0] = assign;
  }

  public static void test()
  {
    GenericArray<Foo> myGenericValue = new GenericArray<Foo>();
    myGenericValue.t = null;
    processSimpleGeneric(myGenericValue);

    myGenericValue.t = new Foo[1];
    myGenericValue.t[0] = new Foo();
    myGenericValue.t[0].x = 4;
    processUpperBoundInterfaceGeneric(myGenericValue);

    GenericArray<Bar> anotherGenericValue = new GenericArray<Bar>();
    anotherGenericValue.t = new Bar[1];
    anotherGenericValue.t[0] = new Bar();
    anotherGenericValue.t[0].x = 4;
    processUpperBoundClassGeneric(anotherGenericValue);

    GenericArray<Object> baseGenericValue = new GenericArray<Object>();
    baseGenericValue.t = new Object[1];
    processLowerBoundGeneric(baseGenericValue, new Foo());
  }
}