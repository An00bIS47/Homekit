# Get Reference Time

Get the reference time used for fakegato

**URL** : `/api/reftime`

**Method** : `GET`

**Auth required** : YES


## Success Response

**Code** : `200 OK`

**Content example**

```json
{
    "reftime": 12345678
}
```

## Error Response

**Condition** : If 'username' and 'password' combination is wrong.

**Code** : `401 Forbidden`