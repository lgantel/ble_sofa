package com.example.wifi_sofa_app

/**
 * A sealed class restricts the types that can inherit from it, allowing the
 * compiler to know all possible subtypes. This is perfect for representing
 * different kinds of API requests.
 */
/**
 * A sealed class to represent all possible POST request bodies.
 * Using a sealed class ensures that you can handle all post types
 * in a 'when' expression, which makes the code safer and more readable.
 */
sealed class PostRequest

/**
 * POST request permitting to set the LED value
 * Represents a JSON like: {"led": 1}
 */
data class PostLED(
    /** LED value: 0 or 1 */
    val led: Int
) : PostRequest()

/**
 * Data class for the POST request to set the relay value.
 * Represents a JSON like: {"relay": 1, "dir": "UP"}
 */
data class PostRELAY(
    /** Relay value: 0 or 1 */
    val relay: Int,
    /** Relay direction: 'UP' or 'DOWN' */
    val dir: String
) : PostRequest()
