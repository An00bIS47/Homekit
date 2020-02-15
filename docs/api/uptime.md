# Uptime

Get the uptime of the device in seconds

**URL** : `/api/uptime`

**Method** : `GET`

**Auth required** : YES


## Success Response

**Code** : `200 OK`

**Content example**

```json
{
    "uptime": 123
}
```

## Error Response

**Condition** : If 'username' and 'password' combination is wrong.

**Code** : `401 Forbidden`