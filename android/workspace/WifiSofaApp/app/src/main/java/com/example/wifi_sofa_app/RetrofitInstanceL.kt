package com.example.wifi_sofa_app

import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory

/**
 * Singleton object used to provide a single instance of Retrofit for
 * the entire application
 */
object RetrofitInstanceL {
    private const val BASE_URL = "http://192.168.1.95:80/"

    val api: ApiService by lazy {
        Retrofit.Builder()
            .baseUrl(BASE_URL)
            .addConverterFactory(GsonConverterFactory.create())
            .build()
            .create(ApiService::class.java)
    }
}
