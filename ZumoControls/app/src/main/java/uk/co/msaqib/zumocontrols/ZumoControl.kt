package uk.co.msaqib.zumocontrols

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.os.IBinder
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.util.Log
import android.widget.Toast
import kotlinx.android.synthetic.main.activity_zumo_control.*
import uk.co.msaqib.zumocontrols.Interfaces.IServiceCallbacks
import uk.co.msaqib.zumocontrols.services.ArduinoCommunicationService
import java.io.IOException
import java.util.*
import android.view.View
import android.view.WindowManager
import android.view.WindowManager.*
import org.eclipse.paho.android.service.MqttAndroidClient
import org.eclipse.paho.client.mqttv3.*
import org.eclipse.paho.client.mqttv3.MqttException
import org.eclipse.paho.client.mqttv3.IMqttToken
import org.eclipse.paho.client.mqttv3.IMqttActionListener
import uk.co.msaqib.zumocontrols.custom.ZumoAdapter
import uk.co.msaqib.zumocontrols.custom.ZumoItem
import java.io.UnsupportedEncodingException
import java.lang.Exception


class ZumoControl : AppCompatActivity(), IServiceCallbacks, SensorEventListener
{
    val TAG = "ZUMO-CONTROL"
    private val TOPIC = "zumo/mcu/data"
    private val QOS = 1
    private val BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB") // "random" unique identifier
    private lateinit var mService: ArduinoCommunicationService
    private var mBound: Boolean = false
    private lateinit var mSensorManager: SensorManager
    private var isControlActivated = false
    private var isBTConnectionActive = false
    private var isAutoMode = false;

    private lateinit var recyclerView: RecyclerView
    private lateinit var viewAdapter: ZumoAdapter
    private lateinit var viewManager: RecyclerView.LayoutManager
    private lateinit var connectedZumos: MutableList<ZumoItem>
    private var mPlayerName = ""
    private var deathCount = 0

    /** Defines callbacks for service binding, passed to bindService()  */
    private val connection = object : ServiceConnection {

        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // We've bound to LocalService, cast the IBinder and get LocalService instance
            val binder = service as ArduinoCommunicationService.LocalBinder
            mService = binder.getService()
            mBound = true
            setupConnection()
        }

        override fun onServiceDisconnected(arg0: ComponentName) {
            mBound = false
        }
    }

    override fun onAccuracyChanged(p0: Sensor?, p1: Int) {
        return
    }

    override fun onSensorChanged(event: SensorEvent) {
        if (event.sensor.type == Sensor.TYPE_ACCELEROMETER) {
            handleAccelerometer(event);
        }
    }

    private fun handleAccelerometer(event: SensorEvent) {
        val values = event.values
        // Movement
        val x = values[0]
        val y = values[1]
        val z = values[2]

        val accelationSquareRoot = (x * x + y * y + z * z) / (SensorManager.GRAVITY_EARTH * SensorManager.GRAVITY_EARTH)

        val zAxis = (z - 5)
        var speed = Math.round((zAxis * 80))
        if(speed > 400){
            speed = 400;
        }else if(speed < -400){
            speed = -400;
        }

        var state =
            if(Math.round(y) > 0 && speed > 0)
                "FORWARD|RIGHT"
            else if(Math.round(y) < 0 && speed > 0)
                "FORWARD|LEFT"
            else if(Math.round(y) > 0 && speed < 0)
                "BACKWARD|RIGHT"
            else if(Math.round(y) < 0 && speed < 0)
                "BACKWARD|LEFT"
            else if(speed > 20)
                "FORWARD"
            else if(speed < -20)
                "BACKWARD"
            else
                "STOP"

        var turn_speed = Math.round(400 / (5 - y))

        if(turn_speed > speed){
            turn_speed = Math.round(speed / 2f)
        }
        runOnUiThread {
            tv_z.text = Math.round(z).toString()
            tv_y.text = Math.round(y).toString()
            tv_state.text = state

            when(state){
                "FORWARD"->{
                    tv_motor_left.text = speed.toString()
                    tv_motor_right.text = speed.toString()
                }
                "BACKWARD"->{
                    tv_motor_left.text = speed.toString()
                    tv_motor_right.text = speed.toString()
                }
                "FORWARD|LEFT"->{
                    tv_motor_left.text = turn_speed.toString()
                    tv_motor_right.text = speed.toString()
                }
                "FORWARD|RIGHT"->{
                    tv_motor_left.text = speed.toString()
                    tv_motor_right.text = turn_speed.toString()
                }
                "BACKWARD|LEFT"->{
                    tv_motor_left.text = turn_speed.toString()
                    tv_motor_right.text = speed.toString()
                }
                "BACKWARD|RIGHT"->{
                    tv_motor_left.text = speed.toString()
                    tv_motor_right.text = turn_speed.toString()
                }
            }
        }

        if(::mService.isInitialized && isControlActivated){
            when(state){
                "FORWARD"->{
                    mService.writeToArduino("$state:$speed|$speed;")
                }
                "BACKWARD"->{
                    mService.writeToArduino("$state:$speed|$speed;")
                }
                "FORWARD|LEFT"->{
                    mService.writeToArduino("$state:$turn_speed|$speed;")
                }
                "FORWARD|RIGHT"->{
                    mService.writeToArduino("$state:$speed|$turn_speed;")
                }
                "BACKWARD|LEFT"->{
                    mService.writeToArduino("$state:$turn_speed|$speed;")
                }
                "BACKWARD|RIGHT"->{
                    mService.writeToArduino("$state:$speed|$turn_speed;")
                }
            }
        }
    }

    //Service Callback
    override fun onDataReceived(byteArray: ByteArray) {
        runOnUiThread {
//            receiver.text = ""
//            receiver.text = String(byteArray)
        }
    }

    override fun updateStatus(currentStatus: Boolean, statusText: String) {
        runOnUiThread{
            isBTConnectionActive = currentStatus

            if(currentStatus){
                btnReconnect.visibility = View.GONE
                switchAnimation("anim-bluetooth.json")
            }else{
                btnReconnect.visibility = View.VISIBLE
                btnStartStop.text = "START"
                isControlActivated = false
                switchAnimation("anim-connection-error.json")
            }
            status.text = statusText
        }
    }

    override fun onError(e: String) {
        Log.d("ZUMO-CONTROL", e)
    }

    override fun onStart() {
        super.onStart()
        // Bind to LocalService
        Intent(this, ArduinoCommunicationService::class.java).also { intent ->
            bindService(intent, connection, Context.BIND_AUTO_CREATE)
        }
    }

    override fun onResume() {
        super.onResume()
        mSensorManager.registerListener(this,
            mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
            SensorManager.SENSOR_DELAY_NORMAL);

        val clientId = MqttClient.generateClientId()
        client = MqttAndroidClient(
            this.applicationContext, "tcp://broker.hivemq.com:1883",
            clientId
        )
        try {
            val token = client.connect()
            token.actionCallback = object : IMqttActionListener {
                override fun onSuccess(asyncActionToken: IMqttToken) {
                    runOnUiThread {
                        Toast.makeText(this@ZumoControl, "MQTT Connection Success", Toast.LENGTH_SHORT).show()
                    }

                    try {
                        val subToken = client.subscribe(TOPIC, QOS)
                        subToken.actionCallback = object : IMqttActionListener {
                            override fun onSuccess(asyncActionToken: IMqttToken) {
                                // The message was published
                                runOnUiThread {
                                    Toast.makeText(this@ZumoControl, "Subscribed to $TOPIC", Toast.LENGTH_SHORT).show()
                                }
                            }

                            override fun onFailure(
                                asyncActionToken: IMqttToken,
                                exception: Throwable
                            ) {
                                // The subscription could not be performed, maybe the user was not
                                // authorized to subscribe on the specified topic e.g. using wildcards

                            }
                        }
                    } catch (e: MqttException) {
                        e.printStackTrace()
                    }
                    // We are connected
                    Log.d(TAG, "onSuccess")
                }

                override fun onFailure(asyncActionToken: IMqttToken, exception: Throwable) {
                    runOnUiThread {
                        Toast.makeText(this@ZumoControl, "MQTT Connection Failed ${exception.message}", Toast.LENGTH_SHORT).show();
                    }

                    // Something went wrong e.g. connection timeout or firewall problems
                    Log.d(TAG, exception.localizedMessage)
                    exception.printStackTrace()
                }
            }
        } catch (e: MqttException) {
            e.printStackTrace()
        } catch (e: Exception){
            e.printStackTrace()
        }

        if(::client.isInitialized){
           Thread{
               client.setCallback(object: MqttCallback{
                   override fun messageArrived(topic: String?, message: MqttMessage?) {
                       Log.d("MQTT-CALLBACK-NEW-MESS", message.toString())

                       val incoming = message.toString().replace("\n", "").replace("\r", "");

                       if(incoming.contains('|')){
                           try{
                               val parts = incoming.split('|');

                               if(parts[1].contains('=')){
                                   val name = parts[0]
                                   val subParts = parts[1].split('=')
                                   var status = subParts[0]
                                   val health = subParts[1]

                                   if(connectedZumos.stream().anyMatch { z -> z.name.equals(name) }){
                                       val item = connectedZumos.find { e -> e.name.equals(name) }
                                       if(item != null){
                                           item.life = health;
                                           connectedZumos[connectedZumos.indexOf(item)] = item
                                       }
                                   }else if(!name.isEmpty() && health.toInt() > 0){
                                       val item = ZumoItem(name,health);
                                       connectedZumos.add(item);

                                       for (zumo in connectedZumos){
                                           publishMessageToZumo("${zumo.name}|HEALTH=${zumo.life}")
                                       }
                                   }

                                   runOnUiThread {
                                       viewAdapter.updateData(connectedZumos)
                                   }
                               }else{
                                   runOnUiThread {
                                       Toast.makeText(this@ZumoControl, "${parts[0]} Connected", Toast.LENGTH_SHORT).show()
                                   }
                               }
                           }catch (e: Exception){
                               runOnUiThread {
                                   Toast.makeText(this@ZumoControl, e.localizedMessage, Toast.LENGTH_SHORT).show()
                               }
                           }
                       }

                   }

                   override fun connectionLost(cause: Throwable?) {
                       Log.d("MQTT-CALLBACK", "Connection Lost")
                   }

                   override fun deliveryComplete(token: IMqttDeliveryToken?) {
                       Log.d("MQTT-CALLBACK-DEL-CMPT", "delivery Completed")
                   }

               });
           }.start()
        }
    }

    private fun publishMessageToZumo(payload: String){
        val encodedPayload: ByteArray
        try {
            encodedPayload = payload.toByteArray(Charsets.UTF_8)
            val message = MqttMessage(encodedPayload)
            client.publish(TOPIC, message);
        } catch (e: MqttException) {
            e.printStackTrace();
        } catch (e: UnsupportedEncodingException){
            e.printStackTrace();
        } catch (e: Exception){
            e.printStackTrace()
        }
    }

    override fun onStop() {
        super.onStop()
        mService.stopZumo();
        mService.closeConnection()
        unbindService(connection)
        mBound = false
    }

    override fun onDestroy() {
        super.onDestroy()
        window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
    }

    override fun onPause() {
        super.onPause()
        mService.stopZumo()
        mSensorManager.unregisterListener(this)

        try {
            client.unsubscribe(TOPIC)
            client.disconnect()
        } catch (e: MqttException) {
            e.printStackTrace()
        }catch (e: Exception){
            e.printStackTrace()
        }

    }

    private lateinit var client: MqttAndroidClient;

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_zumo_control)
        window.addFlags(LayoutParams.FLAG_KEEP_SCREEN_ON)

        mPlayerName = intent.getStringExtra("zumo.player.name")
        connectedZumos = mutableListOf()
        mSensorManager = getSystemService(SENSOR_SERVICE) as SensorManager

        btnStartStop.setOnClickListener {
            Thread{
                if(isBTConnectionActive){
                    if(isControlActivated)
                        mService.stopZumo()
                    isControlActivated = !isControlActivated
                    runOnUiThread {
                        btnStartStop.text = if(isControlActivated) "STOP" else "START"
                    }
                }else{
                    runOnUiThread {
                        Toast.makeText(this, "No Connection", Toast.LENGTH_SHORT).show()
                    }
                }
            }.start()
        }

        btnMode.setOnClickListener {
            Thread{
                if(isBTConnectionActive){
                    if(::mService.isInitialized){
                        if(isAutoMode){
                            mService.stopZumo()
                        }else{
                            mService.startAutoMode()
                        }

                        isAutoMode = !isAutoMode
                        runOnUiThread {
                            btnMode.text = if(isAutoMode) "Stop Auto" else "Auto Mode"
                        }
                    }
                }else{
                    runOnUiThread {
                        Toast.makeText(this, "No Connection", Toast.LENGTH_SHORT).show()
                    }
                }
            }.start()
        }

        btnReconnect.setOnClickListener {
            val statustext = "Connecting..."
            Toast.makeText(this, statustext, Toast.LENGTH_SHORT).show()
            status.text = statustext
            setupConnection()
        }

        viewManager = LinearLayoutManager(this)
        viewAdapter = ZumoAdapter(connectedZumos, this)

        recyclerView = rv_zumolist.apply {
            // use this setting to improve performance if you know that changes
            // in content do not change the layout size of the RecyclerView
            setHasFixedSize(true)

            // use a linear layout manager
            layoutManager = viewManager

            // specify an viewAdapter (see also next example)
            adapter = viewAdapter

        }

    }

    private fun switchAnimation(file: String){
        av_status.setAnimation(file)
        av_status.playAnimation()
        av_status.loop(true)
    }

    fun setupConnection(){
        switchAnimation("anim-loading.json")
        if(mBound){
            val address = intent.getStringExtra("bt.device.address")
            val mBTAdapter = BluetoothAdapter.getDefaultAdapter() // get a handle on the bluetooth radio
            Toast.makeText(this, "Connecting...", Toast.LENGTH_SHORT).show()

            // Spawn a new thread to avoid blocking the GUI one
            Thread{
                val device = mBTAdapter.getRemoteDevice(address)

                try {
                    val mBTSocket = createBluetoothSocket(device)
                    mService.startBTConnection(mBTSocket, this@ZumoControl,mPlayerName)
                    mService.writeToArduino("PLAYER-NAME:$mPlayerName;")
                } catch (e: IOException) {
                    Toast.makeText(this@ZumoControl, "Socket creation failed", Toast.LENGTH_SHORT).show()
                }

            }.start()
        }
    }

    @Throws(IOException::class)
    private fun createBluetoothSocket(device: BluetoothDevice): BluetoothSocket {
        return device.createRfcommSocketToServiceRecord(BTMODULEUUID)
        //creates secure outgoing connection with BT device using UUID
    }
}
