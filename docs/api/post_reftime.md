# Set Reference Time

Set the reference time used for fakegato.

Requires a restart to take effect!

**URL** : `/api/reftime`

**Method** : `POST`

**Auth required** : YES

**Data example**
```json
{
    "reftime": 12345678
}
```

## Success Response

**Code** : `201 Created`

## Error Response

**Condition** : If 'username' and 'password' combination is wrong.

**Code** : `401 Forbidden`