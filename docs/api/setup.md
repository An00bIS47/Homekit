# Setup

Get the pin and xhm code for pairing

**URL** : `/api/setup`

**Method** : `GET`

**Auth required** : YES


## Success Response

**Code** : `200 OK`

**Content example**

```json
{
  "pin": "031-45-712",
  "xhm": "X-HM://0023ISZCGUPFT"
}
```

## Error Response

**Condition** : If 'username' and 'password' combination is wrong.

**Code** : `401 Forbidden`