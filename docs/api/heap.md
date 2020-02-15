# Heap

Get the free heap of the device

**URL** : `/api/heap`

**Method** : `GET`

**Auth required** : YES


## Success Response

**Code** : `200 OK`

**Content example**

```json
{
    "heap": 134500
}
```

## Error Response

**Condition** : If 'username' and 'password' combination is wrong.

**Code** : `401 Forbidden`