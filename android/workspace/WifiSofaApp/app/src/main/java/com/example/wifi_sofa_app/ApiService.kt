package com.example.wifi_sofa_app

import retrofit2.Response
import retrofit2.http.Body
import retrofit2.http.POST

interface ApiService {
    @POST("api/led-control") // "api/led-control" is the endpoint
    suspend fun createPostLED(@Body post: PostLED): Response<PostLED>

    @POST("api/relay-control")
    suspend fun createPostRELAY(@Body post: PostRELAY): Response<PostRELAY>
}
