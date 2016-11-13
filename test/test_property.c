#include "test_common.h"

void test_property_load() {
    Property prop;
    Property *propPtr = &prop;
    uint8_t buffer[FDB_PROPERTY_DISKSIZE];

    uint64_t v64;

    /* test void */
    buffer[0] = DATATYPE_VOID;
    v64 = 0;
    memcpy(buffer+1, &v64, 8);

    fdb_property_load(&prop, buffer);

    fdb_assert("Data type is not void", prop.dataType == DATATYPE_VOID);
    fdb_assert("Value is not 0", memcmp(&v64, prop.data, 8) == 0);
    fdb_assert("Is not void", (fdb_property_isvoid(propPtr)));
    fdb_assert("Is boolean", !(fdb_property_isboolean(propPtr)));
    fdb_assert("Is numeric", !(fdb_property_isnumeric(propPtr)));
    fdb_assert("Is string", !(fdb_property_isstring(propPtr)));
    fdb_assert("Is reference", !(fdb_property_isreference(propPtr)));

    /* test boolean */
    buffer[0] = DATATYPE_FALSE;
    fdb_property_load(&prop, buffer);

    fdb_assert("Data type is not false", prop.dataType == DATATYPE_FALSE);
    fdb_assert("Value is not 0", memcmp(&v64, prop.data, 8) == 0);
    fdb_assert("Is void", !(fdb_property_isvoid(propPtr)));
    fdb_assert("Is not boolean", (fdb_property_isboolean(propPtr)));
    fdb_assert("Is numeric", !(fdb_property_isnumeric(propPtr)));
    fdb_assert("Is string", !(fdb_property_isstring(propPtr)));
    fdb_assert("Is reference", !(fdb_property_isreference(propPtr)));

    buffer[0] = DATATYPE_TRUE;
    fdb_property_load(&prop, buffer);

    fdb_assert("Data type is not true", prop.dataType == DATATYPE_TRUE);
    fdb_assert("Value is not 0", memcmp(&v64, prop.data, 8) == 0);
    fdb_assert("Is void", !(fdb_property_isvoid(propPtr)));
    fdb_assert("Is not boolean", (fdb_property_isboolean(propPtr)));
    fdb_assert("Is numeric", !(fdb_property_isnumeric(propPtr)));
    fdb_assert("Is string", !(fdb_property_isstring(propPtr)));
    fdb_assert("Is reference", !(fdb_property_isreference(propPtr)));

    /* test integer */
    buffer[0] = DATATYPE_INTEGER;
    v64 = 1234;
    memcpy(buffer+1, &v64, 8);

    fdb_property_load(&prop, buffer);

    fdb_assert("Data type is not integer", prop.dataType == DATATYPE_INTEGER);
    fdb_assert("Value is not 1234", memcmp(&v64, prop.data, 8) == 0);
    fdb_assert("Is void", !(fdb_property_isvoid(propPtr)));
    fdb_assert("Is boolean", !(fdb_property_isboolean(propPtr)));
    fdb_assert("Is not numeric", (fdb_property_isnumeric(propPtr)));
    fdb_assert("Is string", !(fdb_property_isstring(propPtr)));
    fdb_assert("Is reference", !(fdb_property_isreference(propPtr)));

    fdb_passed;

    /* test document */
    buffer[0] = DATATYPE_DOCUMENT;
    v64 = 1234;
    memcpy(buffer+1, &v64, 8);

    fdb_property_load(&prop, buffer);

    fdb_assert("Data type is not document", prop.dataType == DATATYPE_DOCUMENT);
    fdb_assert("Value is not 1234", memcmp(&v64, prop.data, 8) == 0);
    fdb_assert("Is void", !(fdb_property_isvoid(propPtr)));
    fdb_assert("Is boolean", !(fdb_property_isboolean(propPtr)));
    fdb_assert("Is numeric", !(fdb_property_isnumeric(propPtr)));
    fdb_assert("Is string", !(fdb_property_isstring(propPtr)));
    fdb_assert("Is not reference", (fdb_property_isreference(propPtr)));

    fdb_passed;
}

void test_property_unload() {
    Property prop;
    uint8_t buffer[FDB_PROPERTY_DISKSIZE];

    uint64_t v64;

    prop.dataType = DATATYPE_DATE;
    v64 = 1234;
    memcpy(prop.data, &v64, 8);

    fdb_property_unload(&prop, buffer);

    fdb_assert("Buffer's type is not date", buffer[0] == DATATYPE_DATE);
    fdb_assert("Buffer's value is not correct", memcmp(&v64, buffer+1, 8) == 0);

    fdb_passed;
}

void test_property_tobool() {
    Property prop;

    prop.dataType = DATATYPE_FALSE;
    fdb_assert("False is not false", fdb_property_tobool(&prop) == 0);

    prop.dataType = DATATYPE_TRUE;
    fdb_assert("True is not true", fdb_property_tobool(&prop) == 1);

    prop.dataType = DATATYPE_STRING;
    fdb_assert("String is not false", fdb_property_tobool(&prop) == 0);

    fdb_passed;
}

void test_property_toi64() {
    Property prop;

    int64_t v;

    v = htolei64(1234);
    memcpy(prop.data, &v, 8);

    prop.dataType = DATATYPE_INTEGER;
    fdb_assert("Integer value was not correct", fdb_property_toi64(&prop) == 1234);

    prop.dataType = DATATYPE_DATE;
    fdb_assert("Date value was not correct", fdb_property_toi64(&prop) == 1234);

    prop.dataType = DATATYPE_ARRAY;
    fdb_assert("Array value was not correct", fdb_property_toi64(&prop) == 0);

    fdb_passed;
}

void test_property_tou64() {
    Property prop;

    uint64_t v;

    v = htolei64(1234);
    memcpy(prop.data, &v, 8);

    prop.dataType = DATATYPE_INTEGER;
    fdb_assert("Integer value was not correct", fdb_property_tou64(&prop) == 0);

    prop.dataType = DATATYPE_DATE;
    fdb_assert("Date value was not correct", fdb_property_tou64(&prop) == 0);

    prop.dataType = DATATYPE_ARRAY;
    fdb_assert("Array value was not correct", fdb_property_tou64(&prop) == 1234);

    prop.dataType = DATATYPE_DOCUMENT;
    fdb_assert("Document value was not correct", fdb_property_tou64(&prop) == 1234);

    prop.dataType = DATATYPE_STRING;
    fdb_assert("String value was not correct", fdb_property_tou64(&prop) == 1234);

    prop.dataType = DATATYPE_BLOB;
    fdb_assert("Blob value was not correct", fdb_property_tou64(&prop) == 1234);

    prop.dataType = DATATYPE_SYMBOL;
    fdb_assert("Symbol value was not correct", fdb_property_tou64(&prop) == 0);

    fdb_passed;
}

void test_property_tof64() {
    Property prop;

    double ov;
    double v;

    v = htolef64(ov);
    memcpy(prop.data, &v, 8);

    prop.dataType = DATATYPE_REAL;
    fdb_assert("Real value was not correct", fdb_property_tof64(&prop) == ov);

    prop.dataType = DATATYPE_DATE;
    fdb_assert("Date value was not correct", fdb_property_tof64(&prop) == 0);

    prop.dataType = DATATYPE_ARRAY;
    fdb_assert("Array value was not correct", fdb_property_tof64(&prop) == 0);

    fdb_passed;
}

void test_property_tou32() {
    Property prop;

    uint32_t ov = 1234;
    uint32_t v;

    v = htoleu32(ov);
    memcpy(prop.data, &v, 4);

    prop.dataType = DATATYPE_SYMBOL;
    fdb_assert("Symbol value was not correct", fdb_property_tou32(&prop) == ov);

    prop.dataType = DATATYPE_DATE;
    fdb_assert("Date value was not correct", fdb_property_tou32(&prop) == 0);

    prop.dataType = DATATYPE_ARRAY;
    fdb_assert("Array value was not correct", fdb_property_tou32(&prop) == 0);

    fdb_passed;
}

void test_property_toi32() {
    Property prop;

    int32_t ov = 1234;
    int32_t v;

    v = htolei32(ov);
    memcpy(prop.data, &v, 4);

    prop.dataType = DATATYPE_UCHAR;
    fdb_assert("Uchar value was not correct", fdb_property_toi32(&prop) == ov);

    prop.dataType = DATATYPE_DATE;
    fdb_assert("Date value was not correct", fdb_property_toi32(&prop) == 0);

    prop.dataType = DATATYPE_ARRAY;
    fdb_assert("Array value was not correct", fdb_property_toi32(&prop) == 0);

    fdb_passed;
}

void test_property_toratio() {
    Property prop;
    ratio r;

    int32_t n = htolei32(1234);
    int32_t d = htolei32(12);

    memcpy(prop.data, &n, 4);
    memcpy(prop.data + 4, &d, 4);

    prop.dataType = DATATYPE_RATIO;
    r = fdb_property_toratio(&prop);
    fdb_assert("Ratio numerator was not correct", r.numer == 1234);
    fdb_assert("Ratio denominator was not correct", r.denom == 12);

    prop.dataType = DATATYPE_DATE;
    r = fdb_property_toratio(&prop);
    fdb_assert("Date numerator was not correct", r.numer == 0);
    fdb_assert("Date denominator was not correct", r.denom == 0);

    fdb_passed;
}

void test_labeled_property_load() {
    LabeledProperty p;
    uint8_t buffer[13];

    int64_t value = htolei64(23456);
    uint32_t symbolid = htoleu32(1234);

    memcpy(buffer, &symbolid, 4);

    buffer[4] = DATATYPE_INTEGER;
    memcpy(buffer + 5, &value, 8);

    fdb_labeledproperty_load(&p, buffer);

    fdb_assert("Label id was not set correctly", p.labelId == 1234);
    fdb_assert("Type not correct", p.prop.dataType == DATATYPE_INTEGER);
    fdb_assert("Value not correct", memcmp(p.prop.data, &value, 8) == 0);

    fdb_passed;
}

void test_labeled_property_unload() {
    LabeledProperty p;
    uint8_t buffer[13];

    int64_t value = htolei64(23456);
    uint32_t symbolid = htoleu32(1234);

    p.labelId = 1234;
    p.prop.dataType = DATATYPE_INTEGER;
    memcpy(p.prop.data, &value, 8);

    fdb_labeledproperty_unload(&p, buffer);

    fdb_assert("Label id was not unloaded correctly", memcmp(buffer, &symbolid, 4) == 0);
    fdb_assert("Data type was not unloaded correctly", buffer[4] == DATATYPE_INTEGER);
    fdb_assert("Data value was not unloaded correctly", memcmp(buffer+5, &value, 8) == 0);

    fdb_passed;

}

void test_property() {
    fdb_runtest("property load", test_property_load);
    fdb_runtest("property unload", test_property_unload);
    fdb_runtest("property to bool", test_property_tobool);
    fdb_runtest("property to i64", test_property_toi64);
    fdb_runtest("property to u64", test_property_tou64);
    fdb_runtest("property to f64", test_property_tof64);
    fdb_runtest("property to u32", test_property_tou32);
    fdb_runtest("property to i32", test_property_toi32);
    fdb_runtest("property to ratio", test_property_toratio);
    fdb_runtest("labeled property load", test_labeled_property_load);
    fdb_runtest("labeled property unload", test_labeled_property_unload);

}
