package com.example.wifi_sofa_app

import android.graphics.Color
import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.example.wifi_sofa_app.databinding.FragmentFirstBinding

import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import android.util.Log

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

/**
 * A simple [Fragment] subclass as the default destination in the navigation.
 */
class FirstFragment : Fragment() {

    private var _binding: FragmentFirstBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        _binding = FragmentFirstBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        // Initialize toggle buttons OFF color
        binding.toggleButtonUpL.setBackgroundColor(Color.LTGRAY)
        binding.toggleButtonDownL.setBackgroundColor(Color.LTGRAY)
        binding.toggleButtonUpR.setBackgroundColor(Color.LTGRAY)
        binding.toggleButtonDownR.setBackgroundColor(Color.LTGRAY)

        // Left side of the sofa
        binding.toggleButtonUpL.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) { // New button state
                // Check if the Down relay is already enabled
                if (binding.toggleButtonDownL.isChecked) {
                    // Disable it before enabling the Up relay
                    disableRelayDown("LEFT")
                    binding.toggleButtonDownL.isChecked = false
                }
                enableRelayUp("LEFT")
            } else {
                disableRelayUp("LEFT")
            }
        }

        binding.toggleButtonDownL.setOnCheckedChangeListener { _,  isChecked ->
            if (isChecked) {
                // Check if the Up relay is already enabled
                if (binding.toggleButtonUpL.isChecked) {
                    // Disable it before enabling the Down relay
                    disableRelayUp("LEFT")
                    binding.toggleButtonUpL.isChecked = false
                }
                enableRelayDown("LEFT")
            } else {
                disableRelayDown("LEFT")
            }
        }

        // Right side of the sofa
        binding.toggleButtonUpR.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) { // New button state
                // Check if the Down relay is already enabled
                if (binding.toggleButtonDownR.isChecked) {
                    // Disable it before enabling the Up relay
                    disableRelayDown("RIGHT")
                    binding.toggleButtonDownR.isChecked = false
                }
                enableRelayUp("RIGHT")
            } else {
                disableRelayUp("RIGHT")
            }
        }

        binding.toggleButtonDownR.setOnCheckedChangeListener { _,  isChecked ->
            if (isChecked) {
                // Check if the Up relay is already enabled
                if (binding.toggleButtonUpR.isChecked) {
                    // Disable it before enabling the Down relay
                    disableRelayUp("RIGHT")
                    binding.toggleButtonUpR.isChecked = false
                }
                enableRelayDown("RIGHT")
            } else {
                disableRelayDown("RIGHT")
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

    private fun enableRelayUp(sofaLoc: String) {
        when (sofaLoc) {
            "LEFT" -> binding.toggleButtonUpL.setBackgroundColor(Color.GREEN)
            "RIGHT" -> binding.toggleButtonUpR.setBackgroundColor(Color.GREEN)
            else -> Log.d("FirstFragment", "Unknown Sofa Location")
        }
        sendPost(sofaLoc, "RELAY_UP", 1)
    }

    private fun disableRelayUp(sofaLoc: String) {
        when (sofaLoc) {
            "LEFT" -> binding.toggleButtonUpL.setBackgroundColor(Color.LTGRAY)
            "RIGHT" -> binding.toggleButtonUpR.setBackgroundColor(Color.LTGRAY)
            else -> Log.d("FirstFragment", "Unknown Sofa Location")
        }
        sendPost(sofaLoc, "RELAY_UP", 0)
    }

    private fun enableRelayDown(sofaLoc: String) {
        when (sofaLoc) {
            "LEFT" -> binding.toggleButtonDownL.setBackgroundColor(Color.GREEN)
            "RIGHT" -> binding.toggleButtonDownR.setBackgroundColor(Color.GREEN)
            else -> Log.d("FirstFragment", "Unknown Sofa Location")
        }
        sendPost(sofaLoc, "RELAY_DOWN", 1)
    }

    private fun disableRelayDown(sofaLoc: String) {
        when (sofaLoc) {
            "LEFT" -> binding.toggleButtonDownL.setBackgroundColor(Color.LTGRAY)
            "RIGHT" -> binding.toggleButtonDownR.setBackgroundColor(Color.LTGRAY)
            else -> Log.d("FirstFragment", "Unknown Sofa Location")
        }
        sendPost(sofaLoc, "RELAY_DOWN", 0)
    }

    fun sendPost(sofaLoc: String, endpointType: String, value: Int) {
        // Use lifecycleScope to launch a coroutine
        lifecycleScope.launch {
            // Create a Post object with the data to be sent
            val requestBody: PostRequest = when (endpointType) {
                "LED" -> PostLED(value)
                "RELAY_UP" -> PostRELAY(value, "UP")
                "RELAY_DOWN" -> PostRELAY(value, "DOWN")
                else -> {
                    Log.d("FirstFragment", "Unknow endpoint type")
                    return@launch // Exit coroutine is type is unknown
                }
            }

            try {
                // Make the network request
                when (requestBody) {
                    is PostLED -> {
                        val response = when (sofaLoc) {
                            "LEFT" -> RetrofitInstanceL.api.createPostLED(requestBody)
                            "RIGHT" -> RetrofitInstanceR.api.createPostLED(requestBody)
                            else -> {
                                Log.d("FirstFragment", "Unknow sofa location")
                                return@launch // Exit coroutine is type is unknown
                            }
                        }
                        handleResponse(
                            response.isSuccessful,
                            response.body()?.toString(),
                            response.errorBody()?.toString(),
                            response.code()
                        )
                    }
                    is PostRELAY -> {
                        val response = when (sofaLoc) {
                            "LEFT" -> RetrofitInstanceL.api.createPostRELAY(requestBody)
                            "RIGHT" -> RetrofitInstanceR.api.createPostRELAY(requestBody)
                            else -> {
                                Log.d("FirstFragment", "Unknow sofa location")
                                return@launch // Exit coroutine is type is unknown
                            }
                        }
                        handleResponse(
                            response.isSuccessful,
                            response.body()?.toString(),
                            response.errorBody()?.toString(),
                            response.code()
                        )
                    }
                }
            } catch (e: Exception) {
                handleException(e)
            }
        }
    }

    private suspend fun handleResponse(isSuccessful: Boolean, body: String?, errorBody: String?, code: Int) {
        withContext(Dispatchers.Main) {
            if (isSuccessful && body != null) {
                val responseMessage = "Post created successfully:\n\n$body"
                Log.d("FirstFragment", responseMessage) // Corrected "FistFragment"
                showDialog("Request Successful", responseMessage)
            } else {
                val errorMessage = "Error creating post: $code\n${errorBody ?: "Unknown error"}"
                Log.e("FirstFragment", errorMessage)
                showDialog("Request Failed", errorMessage)
            }
        }
    }

    private suspend fun handleException(e: Exception) {
        withContext(Dispatchers.Main) {
            val exceptionMessage = "Exception: ${e.message}"
            Log.e("FirstFragment", exceptionMessage)
            showDialog("Network Error", exceptionMessage)
        }
    }

    private fun showDialog(title: String, message: String) {
        /*AlertDialog.Builder(requireContext())
            .setTitle(title)
            .setMessage(message)
            .setPositiveButton("OK") { dialog, _ -> dialog.dismiss() }
            .show()*/
    }
}