#include "NumberPatterns.h"

LazyRegex NumberPatterns::naturalNumber("\\d+");
LazyRegex NumberPatterns::integer("[-+]? ?" + naturalNumber);
// LazyRegex NumberPatterns::word("\\w+");
LazyRegex NumberPatterns::unit("%|(?:\\w+)?");

LazyRegex NumberPatterns::unsignedDecimal(naturalNumber + "\\." +
                                          naturalNumber);
LazyRegex NumberPatterns::unsignedPointNumber("\\." + naturalNumber);

LazyRegex NumberPatterns::number(
    "-?" + (naturalNumber | unsignedDecimal | unsignedPointNumber).bracket());

LazyRegex NumberPatterns::numberWithUnit(number.capture() +
                                         LazyRegex::ignoreWhitespace +
                                         unit.capture());
